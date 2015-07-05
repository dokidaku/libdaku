#include "frame-pusher.h"
#include <libavutil/opt.h>

int frame_pusher_open(frame_pusher **o_fp, const char *path,
    int aud_samplerate,
    int vid_framerate, int width, int height, int vid_bitrate)
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
    // >> The audio stream
    AVCodec *aud_codec = avcodec_find_encoder(AV_CODEC_ID_PCM_MULAW);
    fp->aud_stream = avformat_new_stream(fp->fmt_ctx, aud_codec);
    fp->aud_stream->id = 0;
    fp->aud_stream->codec->codec_id = AV_CODEC_ID_PCM_MULAW;
    fp->aud_stream->codec->bit_rate = 64000;
    fp->aud_stream->codec->sample_rate = fp->aud_samplerate = aud_samplerate;
    fp->aud_stream->codec->sample_fmt = AV_SAMPLE_FMT_S16;
    fp->aud_stream->codec->channel_layout = AV_CH_LAYOUT_STEREO;
    fp->aud_stream->codec->channels = 2;
    fp->aud_stream->codec->time_base = fp->aud_stream->time_base = (AVRational){1, aud_samplerate};
    // >> The video stream
    AVCodec *vid_codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    fp->vid_stream = avformat_new_stream(fp->fmt_ctx, vid_codec);
    fp->vid_width = fp->vid_stream->codec->width = width;
    fp->vid_height = fp->vid_stream->codec->height = height;
    fp->vid_stream->id = 1;
    // >>> * ATTENTION: fp->vid_stream->codec is an (AVCodecContext *) rather than (AVCodec *)!
    fp->vid_stream->codec->codec_id = AV_CODEC_ID_H264;
    fp->vid_stream->codec->bit_rate = vid_bitrate > 0 ? vid_bitrate : 1200000;
    fp->vid_stream->codec->pix_fmt = AV_PIX_FMT_YUV420P;
    fp->vid_stream->codec->gop_size = 24;
    fp->vid_stream->codec->time_base = fp->vid_stream->time_base = (AVRational){1, vid_framerate};

    // >> Enable experimental codecs such as AAC
    fp->aud_stream->codec->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;
    fp->vid_stream->codec->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;
    // >> Some formats want stream headers to be separate.
    // >> XXX: MPEG-4 doesn't have AVFMT_GLOBALHEADER in its format flags??
    //if (fp->fmt_ctx->flags & AVFMT_GLOBALHEADER)
        fp->aud_stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
        fp->vid_stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
    if ((ret = avcodec_open2(fp->aud_stream->codec, aud_codec, NULL)) < 0) return ret;
    if ((ret = avcodec_open2(fp->vid_stream->codec, vid_codec, NULL)) < 0) return ret;
    // Trigger a full initialization on the format context and write the header.
    avformat_write_header(fp->fmt_ctx, NULL);

    // Miscellaneous initializations
    fp->first_packet = 1;
    fp->last_aud_pts = fp->last_vid_pts = 0;
    fp->nb_aud_buffered_samples = 0;
    // > Video
    fp->vid_frame = av_frame_alloc();
    fp->pict_bufsize = avpicture_get_size(AV_PIX_FMT_YUV420P, width, height);
    fp->pict_buf = (uint8_t *)av_malloc(fp->pict_bufsize);
    // >> Assign the video frame with the allocated buffer
    avpicture_fill((AVPicture *)fp->vid_frame, fp->pict_buf, AV_PIX_FMT_YUV420P, width, height);
    fp->sws_ctx = sws_getContext(
        width, height, PIX_FMT_RGB24, width, height, AV_PIX_FMT_YUV420P,
        SWS_BILINEAR, NULL, NULL, NULL);
    // > Audio
    fp->aud_frame = av_frame_alloc();
    fp->aud_buf = av_frame_alloc();
    fp->aud_buf->format = fp->aud_frame->format = fp->aud_stream->codec->sample_fmt;
    fp->aud_buf->channel_layout = fp->aud_frame->channel_layout = fp->aud_stream->codec->channel_layout;
    fp->aud_buf->sample_rate = fp->aud_frame->sample_rate = fp->aud_stream->codec->sample_rate;
    if (aud_codec->capabilities & CODEC_CAP_VARIABLE_FRAME_SIZE)
        fp->nb_aud_samples_per_frame = 4096;
    else fp->nb_aud_samples_per_frame = fp->aud_stream->codec->frame_size;
    fp->aud_buf->nb_samples = fp->aud_frame->nb_samples = fp->nb_aud_samples_per_frame;
    av_log(NULL, AV_LOG_INFO, "frame_pusher: number of samples per frame = %d\n", fp->nb_aud_samples_per_frame);
    if ((ret = av_frame_get_buffer(fp->aud_frame, 0)) < 0) return ret;
    if ((ret = av_frame_get_buffer(fp->aud_buf, 0)) < 0) return ret;
    // >> The audio resampling context
    fp->swr_ctx = swr_alloc();
    if (!fp->swr_ctx) {
        av_log(NULL, AV_LOG_ERROR, "frame_pusher: Cannot initialize audio resampling library"
            "(possibly caused by insufficient memory)\n");
        return AVERROR_UNKNOWN;
    }
    av_opt_set_channel_layout(fp->swr_ctx, "in_channel_layout", fp->aud_stream->codec->channel_layout, 0);
    av_opt_set_channel_layout(fp->swr_ctx, "out_channel_layout", fp->aud_stream->codec->channel_layout, 0);
    av_opt_set_int(fp->swr_ctx, "in_sample_rate", fp->aud_stream->codec->sample_rate, 0);
    av_opt_set_int(fp->swr_ctx, "out_sample_rate", fp->aud_stream->codec->sample_rate, 0);
    av_opt_set_sample_fmt(fp->swr_ctx, "in_sample_fmt", AV_SAMPLE_FMT_S16, 0);
    av_opt_set_sample_fmt(fp->swr_ctx, "out_sample_fmt", fp->aud_stream->codec->sample_fmt, 0);
    if ((ret = swr_init(fp->swr_ctx)) < 0) return ret;

    *o_fp = fp;
    return 0;
}

int frame_pusher_write_video(frame_pusher *fp, uint8_t **rgb_data, int *linesize, int delta_pts)
{
    int ret, got_packet;
    if (!fp->first_packet) av_free_packet(&fp->packet);
    else fp->first_packet = 0;

    //if ((ret = av_frame_make_writable(fp->vid_frame)) < 0) return ret;
    fp->vid_frame->pts = fp->last_vid_pts;
    fp->vid_frame->format = AV_PIX_FMT_YUV420P;
    fp->vid_frame->width = fp->vid_width;
    fp->vid_frame->height = fp->vid_height;
    fp->last_vid_pts += delta_pts;
    // Convert colour space
    // TODO: Scale the data to the size set in the struct?
    sws_scale(fp->sws_ctx, (const uint8_t *const *)rgb_data, linesize,
        0, fp->vid_height, fp->vid_frame->data, fp->vid_frame->linesize);
    // Encode
    memset(&fp->packet, 0, sizeof(fp->packet));
    av_init_packet(&fp->packet);
    if ((ret = avcodec_encode_video2(fp->vid_stream->codec, &fp->packet, fp->vid_frame, &got_packet)) < 0) return ret;
    if (got_packet) {
        // Write the frame.
        av_packet_rescale_ts(&fp->packet, fp->vid_stream->codec->time_base, fp->vid_stream->time_base);
        fp->packet.stream_index = fp->vid_stream->index;
        if ((ret = av_interleaved_write_frame(fp->fmt_ctx, &fp->packet)) < 0) return ret;
    }

    return 0;
}

// Internally-used function
// Write the whole buffered frame (contains fp->nb_aud_samples_per_frame samples) to the output.
int _frame_pusher_write_audio_frame(frame_pusher *fp)
{
    int ret, got_packet;
    if (!fp->first_packet) av_free_packet(&fp->packet);
    else fp->first_packet = 0;

    fp->aud_frame->pts = fp->last_aud_pts;
    fp->last_aud_pts += fp->nb_aud_samples_per_frame;
    // Convert samples from signed 16-bit (S16) format to the desired format
    swr_convert(fp->swr_ctx,
        fp->aud_frame->data, fp->aud_frame->nb_samples,
        (const uint8_t **)fp->aud_buf->data, fp->nb_aud_samples_per_frame);
    // Encode
    memset(&fp->packet, 0, sizeof(fp->packet));
    av_init_packet(&fp->packet);
    if ((ret = avcodec_encode_audio2(fp->aud_stream->codec, &fp->packet, fp->aud_frame, &got_packet)) < 0) return ret;
    if (got_packet) {
        // Write the frame.
        av_packet_rescale_ts(&fp->packet, fp->aud_stream->codec->time_base, fp->aud_stream->time_base);
        fp->packet.stream_index = fp->aud_stream->index;
        if ((ret = av_interleaved_write_frame(fp->fmt_ctx, &fp->packet)) < 0) return ret;
    }

    return 0;
}

int frame_pusher_write_audio(frame_pusher *fp, int16_t lch, int16_t rch)
{
    int idx = (fp->nb_aud_buffered_samples++) << 2;
    *(int16_t *)&fp->aud_buf->data[0][idx + 1] = lch;
    *(int16_t *)&fp->aud_buf->data[0][idx + 3] = rch;
    //*(int16_t *)&fp->aud_buf->data[1][idx] = rch;
    if (fp->nb_aud_buffered_samples == fp->nb_aud_samples_per_frame) {
        fp->nb_aud_buffered_samples = 0;
        return _frame_pusher_write_audio_frame(fp);
    }
    else return 0;
}

unsigned char frame_pusher_audio_catches_audio(frame_pusher *fp)
{
    return av_compare_ts(
        fp->last_vid_pts, fp->vid_stream->codec->time_base,
        fp->last_aud_pts, fp->aud_stream->codec->time_base
    ) >= 0;
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