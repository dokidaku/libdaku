// Example for frame_puller: video snapshots
// $ gcc -c ../frame-puller.c -o frame-puller.o
// $ gcc snapshot.c frame-puller.o -o snapshot `pkg-config --cflags --libs libavformat libswscale`

#include "../frame-puller.h"

// Save a frame to a file in PPM (raw RGB) format.
// The frames are converted to RGB24 format internally by the frame puller
//   so we simply write every byte of the picture data to the file. That's a PPM.
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
    if (argc < 3) {
        printf("Video snapshots based on frame pullers\n");
        printf("Usage: %s <input> <output_pattern> [<width> <height>]\n", argv[0]);
        printf("Extracts images from a video.\n");
        printf("The images will be captured once every 50 frames and are in PPM format.\n");
        printf("The frame numbers and the .ppm suffix will be appended to output_pattern.\n");
        return 1;
    }
    av_register_all();

    int output_w = 0, output_h = 0;
    if (argc >= 5) {
        output_w = atoi(argv[3]);
        output_h = atoi(argv[4]);
    }

    int ret;
    frame_puller *fp;
    if ((ret = frame_puller_open_video(&fp, argv[1], output_w, output_h, PIX_FMT_RGB24)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot open input file\n");
        return ret;
    }

    AVFrame *frame;
    int nb_frames_read = 0;
    char path[256];
    // Continuously get video frames and save a snapshot once every 50 frames.
    while (frame_puller_next_frame(fp, &frame) >= 0) if (++nb_frames_read % 50 == 0) {
        // Save a snapshot
        sprintf(path, "%s-%d.ppm", argv[2], nb_frames_read);
        save_frame(path, frame, fp->output_width, fp->output_height);
    }
    if (nb_frames_read % 50) {
        sprintf(path, "%s-%d.ppm", argv[2], nb_frames_read);
        save_frame(path, frame, fp->output_width, fp->output_height);
    }
    printf("Total frames: %d\nTotal snapshots generated: %d\n",
        nb_frames_read, nb_frames_read / 50 + (nb_frames_read % 50 ? 1 : 0));

    // Release resources
    frame_puller_free(fp);
    return 0;
}
