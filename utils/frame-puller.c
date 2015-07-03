#include "frame-puller.h"

// Internally-used function
// Create a new frame_puller struct and open the given file.
int _frame_puller_new(frame_puller **o_fp, const char *path)
{
    *o_fp = NULL;
    int ret;
    frame_puller *fp = (frame_puller *)av_malloc(sizeof(frame_puller));

    // avformat_open_input will try to free fp->fmt_ctx if it's not set to NULL
    // ... and this possibly causes segment faults.
    fp->fmt_ctx = NULL;
    if ((ret = avformat_open_input(&fp->fmt_ctx, path, NULL, NULL)) < 0) return ret;
    if ((ret = avformat_find_stream_info(fp->fmt_ctx, NULL)) < 0) return ret;
    av_dump_format(fp->fmt_ctx, 0, path, 0);    // For debug use
    fp->target_stream_idx = -1;
    fp->frame = fp->codec_ctx = fp->codec = NULL;
    fp->first_packet = 1;

    *o_fp = fp;
    return 0;
}

int frame_puller_open_video(frame_puller **o_fp, const char *path)
{
    *o_fp = NULL;
    int ret;
    frame_puller *fp;

    if ((ret = _frame_puller_new(&fp, path)) < 0) return ret;
    fp->type = FRAME_PULLER_VIDEO;
    // Find the first video stream
    // TODO: Perhaps some files have two or more video streams?
    unsigned int i;
    for (i = 0; i < fp->fmt_ctx->nb_streams; ++i)
        if (fp->fmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            fp->target_stream_idx = i; break;
        }
    if (fp->target_stream_idx == -1) {
        av_log(NULL, AV_LOG_ERROR, "Cannot find a valid video stream\n");
        return AVERROR_INVALIDDATA;
    }
    // Create the codec context and open codec
    // Seems using the original codec context cause potential problems...?
    if (!(fp->codec = avcodec_find_decoder(fp->fmt_ctx->streams[i]->codec->codec_id))) {
        av_log(NULL, AV_LOG_ERROR, "Cannot find a proper decoder\n");
        return AVERROR_INVALIDDATA;
    }
    fp->codec_ctx = avcodec_alloc_context3(fp->codec);
    if ((ret = avcodec_copy_context(fp->codec_ctx, fp->fmt_ctx->streams[i]->codec)) < 0) return ret;
    if ((ret = avcodec_open2(fp->codec_ctx, fp->codec, NULL)) < 0) return ret;
    // Allocate a frame to store the read data
    fp->frame = av_frame_alloc();

    *o_fp = fp;
    return 0;
}

int frame_puller_next_frame(frame_puller *fp, AVFrame **o_frame)
{
    int ret, frame_complete;
    if (!fp->first_packet) av_free_packet(&fp->packet);
    else fp->first_packet = 0;

read_again:
    // Read until a packet in the target stream appear
    while ((ret = av_read_frame(fp->fmt_ctx, &fp->packet)) >= 0
        && fp->packet.stream_index != fp->target_stream_idx);
    if (ret < 0) return ret;
    // Now we've found a wanted frame. Decode it.
    if (fp->type == FRAME_PULLER_VIDEO) {
        avcodec_decode_video2(fp->codec_ctx, fp->frame, &frame_complete, &fp->packet);
        if (frame_complete) o_frame && (*o_frame = fp->frame);
        else goto read_again;   // XXX: Can this possibly happen?
    }

    return 0;
}

void frame_puller_free(frame_puller *fp)
{
    if (!fp) return;
    if (!fp->first_packet) av_free_packet(&fp->packet);
    if (fp->frame) av_free(fp->frame);
    if (fp->codec_ctx) avcodec_close(fp->codec_ctx);
    if (fp->fmt_ctx) avformat_close_input(&fp->fmt_ctx);
    av_free(fp);
}
