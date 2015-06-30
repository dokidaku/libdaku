// Example for frame_puller: video snapshots
// $ gcc -c ../frame-puller.c -o frame-puller.o `pkg-config --cflags --libs libavformat`
// $ gcc snapshot.c frame-puller.o -o snapshot `pkg-config --cflags --libs libavformat libswscale`

#include "../frame-puller.h"
#include <libswscale/swscale.h>

// Save a frame to a file in PPM (raw RGB) format.
int save_frame(const char *path, AVFrame *frame, int width, int height)
{
    FILE *fp = fopen(path, "wb");
    if (!fp) {
        av_log(NULL, AV_LOG_ERROR,
            "Cannot open output file %s. Please ensure that there is enough space.\n", path);
        return 233333;
    }
    fprintf(fp, "P6\n%d %d\n255\n", width, height);
    int y;
    for (y = 0; y < height; ++y)
        fwrite(frame->data[0] + y * frame->linesize[0], 1, width * 3, fp);
    fclose(fp);
    return 0;
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        printf("Video snapshots based on frame pullers\n");
        printf("Usage: %s <input> <output_pattern>\n", argv[0]);
        printf("Extracts images from a video.\n");
        printf("The images will be captured once every 50 frames and are in PPM format.\n");
        printf("The frame numbers and the .ppm suffix will be appended to output_pattern.\n");
        return 1;
    }
    av_register_all();

    int ret;
    frame_puller *fp;
    if ((ret = frame_puller_open_video(&fp, argv[1])) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot open input file\n");
        return ret;
    }

    AVFrame *frame, *cnvt_frame = av_frame_alloc();
    // The context for converting colour space
    struct SwsContext *sws_ctx = sws_getContext(
        fp->codec_ctx->width, fp->codec_ctx->height, fp->codec_ctx->pix_fmt,
        fp->codec_ctx->width, fp->codec_ctx->height, PIX_FMT_RGB24,
        SWS_BILINEAR, NULL, NULL, NULL);
    // The buffer to put the converted images in
    int buffer_size = avpicture_get_size(
        PIX_FMT_RGB24, fp->codec_ctx->width, fp->codec_ctx->height);
    uint8_t *buffer = (uint8_t *)av_malloc(buffer_size);
    // Assign the buffer with the frame's data field.
    avpicture_fill((AVPicture *)cnvt_frame,
        buffer, PIX_FMT_RGB24, fp->codec_ctx->width, fp->codec_ctx->height);

    int nb_frames_read = 0;
    char path[256];
    // Continuously get video frames and save a snapshot once every 50 frames.
    while (frame_puller_next_frame(fp, &frame) >= 0) if (++nb_frames_read % 50 == 0) {
        // Convert and save
        sws_scale(sws_ctx, (const uint8_t *const *)frame->data,
            frame->linesize, 0, fp->codec_ctx->height,
            cnvt_frame->data, cnvt_frame->linesize);
        sprintf(path, "%s-%d.ppm", argv[2], nb_frames_read);
        save_frame(path, cnvt_frame, fp->codec_ctx->width, fp->codec_ctx->height);
    }
    printf("Total frames: %d\nTotal snapshots generated: %d\n",
        nb_frames_read, nb_frames_read / 50);

    // Release resources
    av_free(cnvt_frame);
    sws_freeContext(sws_ctx);
    frame_puller_free(fp);
    return 0;
}
