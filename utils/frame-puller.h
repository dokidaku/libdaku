#ifndef __FRAME_PULLER_H__
#define __FRAME_PULLER_H__

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>

enum frame_puller_type {
    FRAME_PULLER_AUDIO,
    FRAME_PULLER_VIDEO
};

typedef struct __frame_puller {
    enum frame_puller_type type;    /**< Type of frames that will be pulled. */
    AVFormatContext *fmt_ctx;
    int target_stream_idx;
    AVCodecContext *codec_ctx;
    AVCodec *codec;

    union {
        struct SwsContext *sws_ctx;
        struct SwrContext *swr_ctx;
    } libsw;
    int pict_bufsize;
    uint8_t *pict_buf;
    AVFrame *orig_frame;            /**< The last pulled frame. */
    AVFrame *frame;                 /**< The converted frame. */
    // The packet needn't be freed before reading the first packet.
    unsigned char first_packet;
    AVPacket packet;
} frame_puller;

/**
 * Create a new frame_puller struct with a given video/audio file.
 * This puller only pulls audio frames in frame_puller_next_frame and ignore other frames.
 *
 * @param[out] o_fp  The pointer of the struct generated.
 * @param[in]  path  The path of the video/audio file.
 * @return 0 if succeeded, a negative error code otherwise.
 */
int frame_puller_open_audio(frame_puller **o_fp, const char *path);

/**
 * Create a new frame_puller struct with a given video file.
 * This puller only pulls video frames in frame_puller_next_frame and ignore other frames.
 *
 * @param[out] o_fp  The pointer of the struct generated.
 * @param[in]  path  The path of the video file.
 * @return 0 if succeeded, a negative error code otherwise.
 */
int frame_puller_open_video(frame_puller **o_fp, const char *path);

/**
 * Pull the next targeted frame (video / audio frame depending on the puller's type).
 * The output can be accessed by either the second parameter or fp->frame.
 * For videos, the frames are converted to RGB24 pixel format.
 *
 * @param[in]  fp      The frame_puller to use. Only the frames that match its type will be pulled.
 * @param[out] o_frame The pointer of the frame pulled. May be NULL.
 * @return 0 if succeeded, a negative error code otherwise.
 */
int frame_puller_next_frame(frame_puller *fp, AVFrame **o_frame);

/**
 * Free a frame_puller struct and all resources it used.
 *
 * @param[in] fp  The pointer to the struct to be freed.
 */
void frame_puller_free(frame_puller *fp);

#endif
