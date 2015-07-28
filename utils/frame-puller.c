#include "frame-puller.h"
#include <libavutil/opt.h>

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
    fp->frame = NULL;
    fp->codec_ctx = NULL;
    fp->codec = NULL;
    fp->first_packet = 1;
    // Allocate a frame to store the read data
    fp->orig_frame = av_frame_alloc();

    *o_fp = fp;
    return 0;
}

// Internally-used function
// Initializes a new frame_puller struct with a given media type.
int _frame_puller_init(frame_puller *fp, enum AVMediaType media_type)
{
    int ret;
    // Find the first video stream
    // TODO: Perhaps some files have two or more video streams?
    unsigned int i;
    for (i = 0; i < fp->fmt_ctx->nb_streams; ++i)
        if (fp->fmt_ctx->streams[i]->codec->codec_type == media_type) {
            fp->target_stream_idx = i; break;
        }
    if (fp->target_stream_idx == -1) {
        av_log(NULL, AV_LOG_ERROR, "frame_puller: Cannot find a valid stream\n");
        return AVERROR_INVALIDDATA;
    }
    // Create the codec context and open codec
    // Seems using the original codec context cause potential problems...?
    if (!(fp->codec = avcodec_find_decoder(fp->fmt_ctx->streams[i]->codec->codec_id))) {
        av_log(NULL, AV_LOG_ERROR, "frame_puller: Cannot find a proper decoder\n");
        return AVERROR_INVALIDDATA;
    }
    fp->codec_ctx = avcodec_alloc_context3(fp->codec);
    if ((ret = avcodec_copy_context(fp->codec_ctx, fp->fmt_ctx->streams[i]->codec)) < 0) return ret;
    if ((ret = avcodec_open2(fp->codec_ctx, fp->codec, NULL)) < 0) return ret;
    // Allocate a frame to store the read data
    fp->orig_frame = av_frame_alloc();

    return 0;
}

int frame_puller_open_audio(frame_puller **o_fp, const char *path)
{
    *o_fp = NULL;
    int ret;
    frame_puller *fp;

    if ((ret = _frame_puller_new(&fp, path)) < 0) return ret;
    fp->type = FRAME_PULLER_AUDIO;
    if ((ret = _frame_puller_init(fp, AVMEDIA_TYPE_AUDIO)) < 0) return ret;
    // Initialize the libswresample context for audio resampling.
    // > Create the buffer for the converted frame to store data
    fp->frame = av_frame_alloc();
    fp->frame->format = AV_SAMPLE_FMT_S16P;
    fp->frame->channel_layout = fp->codec_ctx->channel_layout;
    fp->frame->sample_rate = fp->codec_ctx->sample_rate;
    if ((fp->codec->capabilities & CODEC_CAP_VARIABLE_FRAME_SIZE) || !strcmp(fp->codec->name, "pcm_mulaw"))
        fp->frame->nb_samples = 4096;
    else fp->frame->nb_samples = fp->codec_ctx->frame_size;
    av_log(NULL, AV_LOG_INFO, "frame_puller: number of samples per frame = %d\n", fp->frame->nb_samples);
    if ((ret = av_frame_get_buffer(fp->frame, 0)) < 0) return ret;
    // > Create the SwrContext
    fp->libsw.swr_ctx = swr_alloc();
    if (!fp->libsw.swr_ctx) {
        av_log(NULL, AV_LOG_ERROR, "frame_puller: Cannot initialize audio resampling library"
            "(possibly caused by insufficient memory)\n");
        return AVERROR_UNKNOWN;
    }
    // > Provide options for the SwrContext
    av_opt_set_channel_layout(fp->libsw.swr_ctx, "in_channel_layout", fp->codec_ctx->channel_layout, 0);
    av_opt_set_channel_layout(fp->libsw.swr_ctx, "out_channel_layout", fp->codec_ctx->channel_layout, 0);
    av_opt_set_int(fp->libsw.swr_ctx, "in_sample_rate", fp->codec_ctx->sample_rate, 0);
    av_opt_set_int(fp->libsw.swr_ctx, "out_sample_rate", fp->codec_ctx->sample_rate, 0);
    av_opt_set_sample_fmt(fp->libsw.swr_ctx, "in_sample_fmt", fp->codec_ctx->sample_fmt, 0);
    av_opt_set_sample_fmt(fp->libsw.swr_ctx, "out_sample_fmt", AV_SAMPLE_FMT_S16P, 0);
    // > Fully initialize the SwrContext
    if ((ret = swr_init(fp->libsw.swr_ctx)) < 0) return ret;

    *o_fp = fp;
    return 0;
}

int frame_puller_open_video(frame_puller **o_fp, const char *path, int output_width, int output_height, enum AVPixelFormat pix_fmt)
{
    *o_fp = NULL;
    int ret;
    frame_puller *fp;

    if ((ret = _frame_puller_new(&fp, path)) < 0) return ret;
    fp->type = FRAME_PULLER_VIDEO;
    if ((ret = _frame_puller_init(fp, AVMEDIA_TYPE_VIDEO)) < 0) return ret;
    int width = fp->codec_ctx->width, height = fp->codec_ctx->height;
    // Initialize the libswscale context for converting pictures to RGB format.
    fp->pict_bufsize = avpicture_get_size(pix_fmt, width, height);
    fp->pict_buf = (uint8_t *)av_malloc(fp->pict_bufsize);
    fp->frame = av_frame_alloc();
    // > Assign the frame with the allocated buffer
    avpicture_fill((AVPicture *)fp->frame, fp->pict_buf, pix_fmt, width, height);
    fp->output_width = output_width > 0 ? output_width : width;
    fp->output_height = output_height > 0 ? output_height : height;
    fp->libsw.sws_ctx = sws_getContext(
        width, height, fp->codec_ctx->pix_fmt,
        fp->output_width, fp->output_height, pix_fmt,
        SWS_BILINEAR, NULL, NULL, NULL);

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
        avcodec_decode_video2(fp->codec_ctx, fp->orig_frame, &frame_complete, &fp->packet);
        if (frame_complete) {
            // Convert to RGB24 pixel format
            sws_scale(fp->libsw.sws_ctx,
                (const uint8_t *const *)fp->orig_frame->data, fp->orig_frame->linesize,
                0, fp->codec_ctx->height, fp->frame->data, fp->frame->linesize);
            o_frame && (*o_frame = fp->frame);
        } else goto read_again;   // XXX: Can this possibly happen?
    } else if (fp->type == FRAME_PULLER_AUDIO) {
        avcodec_decode_audio4(fp->codec_ctx, fp->orig_frame, &frame_complete, &fp->packet);
        if (frame_complete) {
            // Convert to signed 16-bit planar (S16P) format
            swr_convert(fp->libsw.swr_ctx,
                fp->frame->data, fp->orig_frame->nb_samples,
                (const uint8_t **)fp->orig_frame->data, fp->orig_frame->nb_samples);
            o_frame && (*o_frame = fp->frame);
        } else goto read_again;
    }

    return 0;
}

int frame_puller_seek(frame_puller *fp, float time)
{
    int ret;
    AVRational *tb = &fp->fmt_ctx->streams[fp->target_stream_idx]->time_base;
    int64_t timestamp = (int64_t)(time * (float)tb->den / (float)tb->num);
    if ((ret = av_seek_frame(fp->fmt_ctx, fp->target_stream_idx, timestamp, AVSEEK_FLAG_BACKWARD)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "FFmpeg internal error while seeking\n");
        return ret;
    }
    do {
        if ((ret = frame_puller_next_frame(fp, NULL)) < 0) {
            av_log(NULL, AV_LOG_ERROR, "Cannot seek precisely\n");
            return ret;
        }
    } while (fp->packet.pts < timestamp);
    return 0;
}

void frame_puller_free(frame_puller *fp)
{
    if (!fp) return;
    if (!fp->first_packet) av_free_packet(&fp->packet);
    if (fp->orig_frame) av_free(fp->orig_frame);
    if (fp->frame) av_free(fp->frame);
    if (fp->codec_ctx) avcodec_close(fp->codec_ctx);
    if (fp->fmt_ctx) avformat_close_input(&fp->fmt_ctx);
    av_free(fp);
}
