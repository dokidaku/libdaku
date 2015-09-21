#ifndef __FRAME_PUSHER_H__
#define __FRAME_PUSHER_H__

#ifdef __cplusplus
extern "C" {
#endif
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#ifdef __cplusplus
}
#endif

typedef struct __frame_pusher {
    AVFormatContext *fmt_ctx;
    AVStream *aud_stream, *vid_stream;
    int vid_width, vid_height;
    int aud_samplerate;

    struct SwsContext *sws_ctx;
    struct SwrContext *swr_ctx;
    int pict_bufsize;
    uint8_t *pict_buf;  /**< The buffer for storing converted data. Equals to aud_frame->data. */
    AVFrame *aud_frame; /**< The last written audio frame. */
    AVFrame *aud_buf;   /**< The buffer for storing raw S16 audio data. */
    AVFrame *vid_frame; /**< The last written video frame. */
    // The packet needn't be freed before reading the first packet.
    unsigned char first_packet;
    AVPacket packet;    /**< The last written packet. */
    int64_t last_aud_pts, last_vid_pts;
    int nb_aud_buffered_samples, nb_aud_samples_per_frame;
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
 * @param[in]  vid_bitrate   The bit rate of the output video. Automatically fills if set to 0.
 * @return 0 if succeeded, a negative error code otherwise.
 */
int frame_pusher_open(frame_pusher **o_fp, const char *path,
    int aud_samplerate,
    int vid_framerate, int width, int height, int vid_bitrate);

/**
 * Write one frame of video to the output.
 *
 * @param[in] fp        The frame_pusher to use. The frame will be written according to parameters here.
 * @param[in] rgb_data  The RGB representation of the picture. A buffer of size (height * linesize).
 * @param[in] linesize  The size of memory (in bytes) of each line of pixels. Usually a bit larger than (width * bpp).
 * @param[in] delta_pts The increment of Presentation Timestamp.
 *                      Usually set to 1 for each frame since the output's time base is 1 / frame_rate.
 * @return 0 if succeeded, a negative error code otherwise.
 */
int frame_pusher_write_video(frame_pusher *fp, uint8_t *rgb_data, int linesize, int delta_pts);

/**
 * Write one sample of audio to the output.
 *
 * @param[in] fp  The frame_pusher to use. The frame will be written according to parameters here.
 * @param[in] lch The amplitude value of the left channel.
 * @param[in] rch The amplitude value of the right channel.
 * @return 0 if succeeded, a negative error code otherwise.
 */
int frame_pusher_write_audio(frame_pusher *fp, int16_t lch, int16_t rch);

/**
 * Return whether current audio's duration is longer than or equal to the video's.
 *
 * @param[in] fp  The frame_pusher context.
 * @return 1 if current audio's duration is longer than or equal to the video's, 0 otherwise.
 */
unsigned char frame_pusher_audio_catches_audio(frame_pusher *fp);

/**
 * Close a frame_pusher, write the file trailer and release all resources it used.
 *
 * @param[in] fp  The pointer to the struct to be closed.
 */
void frame_pusher_close(frame_pusher *fp);

#endif
