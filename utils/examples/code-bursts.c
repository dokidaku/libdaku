// Example for frame_pusher: a pure-code-generated video
// $ gcc -c ../frame-pusher.c -o frame-pusher.o
// $ gcc code-bursts.c frame-pusher.o -o code-bursts `pkg-config --cflags --libs libavformat libswscale libswresample`

#include "../frame-pusher.h"

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Code-based video generating based on frame pullers & pushers\n");
        printf("Usage: %s <output>\n", argv[0]);
        printf("Reads a video and outputs it to another format.\n");
        return 1;
    }
    av_register_all();

    int ret;
    frame_pusher *pusher;
    // 44100 Hz / 30 fps, size = 640x400, bitrate = 800000
    // The duration will be 10s
    if ((ret = frame_pusher_open(&pusher, argv[1], 44100, 30, 640, 400, 800000)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot open output\n");
        return ret;
    }

    uint8_t *picture[4] = { 0 };
    int linesize[4] = { 0 }, bufsize = 1024 * 402 * sizeof(uint8_t);
    picture[0] = (uint8_t *)av_malloc(bufsize);
    picture[1] = (uint8_t *)av_malloc(bufsize);
    linesize[0] = 1024 * sizeof(uint8_t);

    // Video frames
    int i, j;
    uint8_t t;
    for (i = 0; i < 300; ++i) {
        t = (int)((double)i / 299.0 * 255.0);
        memset(picture[0], t, bufsize);
        frame_pusher_write_video(pusher, picture, linesize, 1);
        for (j = 0; j < 441000 / 300; ++j)
            frame_pusher_write_audio(pusher, sin(i * 441000 / 300 + j) * 2000, sin(i * 441000 / 300 + j) * 2000);
    }

    // Release resources
    frame_pusher_close(pusher);
    return 0;
}
