#include "frame-pusher.h"

int frame_pusher_open(frame_pusher **o_fp, const char *path,
    int vid_framerate, int width, int height)
{
    *o_fp = NULL;
    int ret;
    frame_pusher *fp = (frame_pusher *)av_malloc(sizeof(frame_pusher));

    // Guess the format
    AVOutputFormat *ofmt = av_guess_format(NULL, path, NULL);
    if (!ofmt) {
        ofmt = av_oformat_next(NULL);   // Use the first format available
        av_log(NULL, AV_LOG_WARNING, "Unsupported container format. Using %s instead.\n", ofmt->name);
        // TODO: Add the extension to the path.
    }
    av_log(NULL, AV_LOG_INFO, "Using format %s\n", ofmt->name);
    // Open output file
    AVIOContext *io_ctx;
    if ((ret = avio_open2(&io_ctx, path, AVIO_FLAG_WRITE, NULL, NULL)) < 0) return ret;
    // Create the format context
    fp->fmt_ctx = avformat_alloc_context();
    fp->fmt_ctx->oformat = ofmt;
    fp->fmt_ctx->pb = io_ctx;
    // > Create the streams. Here we simply create one for video and one for audio.
    AVCodec *codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    fp->vid_stream = avformat_new_stream(fp->fmt_ctx, codec);
    fp->vid_width = fp->vid_stream->codec->width = width;
    fp->vid_height = fp->vid_stream->codec->height = height;
    fp->vid_stream->id = 0;
    fp->vid_stream->codec->codec_id = AV_CODEC_ID_H264;
    fp->vid_stream->codec->bit_rate = 400000;
    fp->vid_stream->codec->pix_fmt = AV_PIX_FMT_YUV420P;
    fp->vid_stream->codec->gop_size = 24;
    fp->vid_stream->codec->time_base = fp->vid_stream->time_base = (AVRational){1, vid_framerate};
    // >> Enable experimental codecs such as AAC
    fp->vid_stream->codec->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;
    // >> Some formats want stream headers to be separate.
    // >> XXX: MPEG-4 doesn't have AVFMT_GLOBALHEADER in its format flags??
    //if (fp->fmt_ctx->flags & AVFMT_GLOBALHEADER)
        fp->vid_stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
    if ((ret = avcodec_open2(fp->vid_stream->codec, codec, NULL)) < 0) return ret;
    // Trigger a full initialization on the format context and write the header.
    avformat_write_header(fp->fmt_ctx, NULL);
    // Miscellaneous initializations
    fp->first_packet = 1;
    fp->last_pts = 0;
    fp->pict_bufsize = avpicture_get_size(AV_PIX_FMT_YUV420P, width, height);
    fp->pict_buf = (uint8_t *)av_malloc(fp->pict_bufsize);
    fp->frame = av_frame_alloc();
    // > Assign the frame with the allocated buffer
    avpicture_fill((AVPicture *)fp->frame, fp->pict_buf, AV_PIX_FMT_YUV420P, width, height);
    fp->sws_ctx = sws_getContext(
        width, height, PIX_FMT_RGB24, width, height, AV_PIX_FMT_YUV420P,
        SWS_BILINEAR, NULL, NULL, NULL);

    *o_fp = fp;
    return 0;
}

int frame_pusher_write_video(frame_pusher *fp, uint8_t **rgb_data, int *linesize, int delta_pts)
{
    int ret, got_packet;
    if (!fp->first_packet) av_free_packet(&fp->packet);
    else fp->first_packet = 0;

    //if ((ret = av_frame_make_writable(fp->frame)) < 0) return ret;
    fp->frame->pts = fp->last_pts;
    fp->frame->format = AV_PIX_FMT_YUV420P;
    fp->frame->width = fp->vid_width;
    fp->frame->height = fp->vid_height;
    fp->last_pts += delta_pts;
    // Convert colour space
    // TODO: Scale the data to the size set in the struct?
    sws_scale(fp->sws_ctx, (const uint8_t *const *)rgb_data, linesize,
        0, fp->vid_height, fp->frame->data, fp->frame->linesize);
    // Encode
    memset(&fp->packet, 0, sizeof(fp->packet));
    av_init_packet(&fp->packet);
    if ((ret = avcodec_encode_video2(fp->vid_stream->codec, &fp->packet, fp->frame, &got_packet)) < 0) return ret;
    if (got_packet) {
        // Write the frame.
        av_packet_rescale_ts(&fp->packet, fp->vid_stream->codec->time_base, fp->vid_stream->time_base);
        fp->packet.stream_index = fp->vid_stream->index;
        if ((ret = av_interleaved_write_frame(fp->fmt_ctx, &fp->packet)) < 0) return ret;
    }

    return 0;
}

void frame_pusher_close(frame_pusher *fp)
{
    if (!fp) return;
    if (fp->fmt_ctx) {
        av_write_trailer(fp->fmt_ctx);
        avformat_free_context(fp->fmt_ctx);
    }
    if (!fp->first_packet) av_free_packet(&fp->packet);
}
