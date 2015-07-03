// Example for frame_puller and frame_pusher: video converting / transcoding
// $ gcc -c ../frame-puller.c -o frame-puller.o `pkg-config --cflags --libs libavformat`
// $ gcc -c ../frame-pusher.c -o frame-pusher.o `pkg-config --cflags --libs libavformat libswscale`
// $ gcc convert.c frame-puller.o frame-pusher.o -o convert `pkg-config --cflags --libs libavformat libswscale`

#include "../frame-puller.h"
#include "../frame-pusher.h"
#include <libswscale/swscale.h>

int main(int argc, char *argv[])
{
    if (argc < 3) {
        printf("Video converting based on frame pullers & pushers\n");
        printf("Usage: %s <input> <output> [<frame_rate>] [<bit_rate>]\n", argv[0]);
        printf("Reads a video and outputs it to another format.\n");
        return 1;
    }
    av_register_all();

    // Parse arguments
    int frame_rate = 0, bit_rate = 0;
    if (argc >= 4) frame_rate = atoi(argv[3]);
    if (argc >= 5) bit_rate = atoi(argv[4]);
    if (frame_rate <= 0) frame_rate = 30;
    if (bit_rate < 0) bit_rate = 0; // Let the frame_pusher automatically fill this

    int ret;
    frame_puller *puller;
    if ((ret = frame_puller_open_video(&puller, argv[1])) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot open input file\n");
        return ret;
    }
    frame_pusher *pusher;
    if ((ret = frame_pusher_open(&pusher, argv[2], frame_rate,
        puller->codec_ctx->width, puller->codec_ctx->height, bit_rate)) < 0)
    {
        av_log(NULL, AV_LOG_ERROR, "Cannot initialize the output file\n");
        return ret;
    }

    AVFrame *frame, *cnvt_frame = av_frame_alloc();
    // The context for converting colour space
    struct SwsContext *sws_ctx = sws_getContext(
        puller->codec_ctx->width, puller->codec_ctx->height, puller->codec_ctx->pix_fmt,
        puller->codec_ctx->width, puller->codec_ctx->height, PIX_FMT_RGB24,
        SWS_BILINEAR, NULL, NULL, NULL);
    // The buffer to put the converted images in
    int buffer_size = avpicture_get_size(
        PIX_FMT_RGB24, puller->codec_ctx->width, puller->codec_ctx->height);
    uint8_t *buffer = (uint8_t *)av_malloc(buffer_size);
    // Assign the buffer with the frame's data field.
    avpicture_fill((AVPicture *)cnvt_frame,
        buffer, PIX_FMT_RGB24, puller->codec_ctx->width, puller->codec_ctx->height);

    int nb_frames_read = 0;
    // Continuously get video frames and convert.
    while (frame_puller_next_frame(puller, &frame) >= 0) {
        ++nb_frames_read;
        // Convert and save
        sws_scale(sws_ctx, (const uint8_t *const *)frame->data,
            frame->linesize, 0, puller->codec_ctx->height,
            cnvt_frame->data, cnvt_frame->linesize);
        if ((ret = frame_pusher_write_video(pusher, cnvt_frame->data, cnvt_frame->linesize, 1)) < 0) {
            av_log(NULL, AV_LOG_ERROR, "Failed to write frame #%d. Quitting T^T\n", nb_frames_read);
            return ret;
        }
    }
    printf("Total frames: %d\n", nb_frames_read);

    // Release resources
    av_free(cnvt_frame);
    sws_freeContext(sws_ctx);
    frame_puller_free(puller);
    frame_pusher_close(pusher);
    return 0;
}
