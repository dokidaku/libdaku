#ifndef __FRAME_PUSHER_H__
#define __FRAME_PUSHER_H__

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

typedef struct __frame_pusher {
    AVFormatContext *fmt_ctx;
    AVStream *aud_stream, *vid_stream;
    int vid_width, vid_height;

    struct SwsContext *sws_ctx;
    int pict_bufsize;
    uint8_t *pict_buf;
    AVFrame *frame;     /**< The last written frame. */
    // The packet needn't be freed before reading the first packet.
    unsigned char first_packet;
    AVPacket packet;    /**< The last written packet. */
    int last_pts;
} frame_pusher;

/**
 * Create a new frame_pusher struct with a given output path. The format will be guessed.
 * The pusher can write video (and audio)[not implemented] to the file.
 *
 * @param[out] o_fp          The pointer of the struct generated.
 * @param[in]  path          The path of the output file.
 * @param[in]  vid_framerate The framerate of the output video.
 * @param[in]  width         The frame width of the output video.
 * @param[in]  height        The frame height of the output video.
 * @return 0 if succeeded, a negative error code otherwise.
 */
int frame_pusher_open(frame_pusher **o_fp, const char *path,
    int vid_framerate, int width, int height);

/**
 * Write one frame of video to the output.
 *
 * @param[in] fp        The frame_pusher to use. The frame will be written according to parameters here.
 * @param[in] rgb_data  The RGB representation of the picture. Only rgb_data[0] will be used.
 * @param[in] linesize  The size of memory (in bytes) of each line of pixels. Usually a bit larger than width * bpp.
 *                      Only linesize[0] will be used in an RGB representation.
 * @param[in] delta_pts The increment of Presentation Timestamp.
 *                      Usually set to 1 for each frame since the output's time base is 1 / frame_rate.
 * @return 0 if succeeded, a negative error code otherwise.
 */
int frame_pusher_write_video(frame_pusher *fp, uint8_t **rgb_data, int *linesize, int delta_pts);

/**
 * Close a frame_pusher, write the file trailer and release all resources it used.
 *
 * @param[in] fp  The pointer to the struct to be closed.
 */
void frame_pusher_close(frame_pusher *fp);

#endif
