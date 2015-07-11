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
    int linesize[4] = { 0 }, bufsize = 640 * 3 * 400 * sizeof(uint8_t);
    picture[0] = (uint8_t *)av_malloc(bufsize);
    picture[1] = (uint8_t *)av_malloc(bufsize);
    linesize[0] = 640 * 3 * sizeof(uint8_t);

    // Generate frames
    int i, j;
    uint8_t grey;
    float loudness;
    for (i = 0; i < 300; ++i) {
        // Video
        for (j = linesize[0] * i; j < linesize[0] * (i + 1); ++j)
            picture[0][j] = random() % 256;
        memset(picture[0] + linesize[0] * (i + 1), i < 150 ? 255 : 0, linesize[0]);
        grey = (int)((double)i / 299.0 * 255.0);
        memset(picture[0] + linesize[0] * (i + 2), grey, bufsize);
        frame_pusher_write_video(pusher, picture, linesize, 1);
        // Audio
        loudness = ((1 + sin((float)i / 75.0 * M_PI)) / 2);
        loudness = loudness * loudness * 2 + 0.2;
        for (j = 0; j < 441000 / 300; ++j)
            frame_pusher_write_audio(pusher,
                (int16_t)(loudness * (float)(random() % 200 - 100)),
                (int16_t)(loudness * (float)(random() % 200 - 100)));
    }

    // Release resources
    frame_pusher_close(pusher);
    return 0;
}
