// Example for frame_pusher: a pure-code-generated video
// $ gcc code-bursts.c ../frame-pusher.c -o code-bursts `pkg-config --cflags --libs libavformat libswscale libswresample`

#include "../frame-pusher.h"
// Ensure VID_HEIGHT > VID_DUR * VID_FRATE since the sweeping line in the video
// moves down one pixel each frame...
#define VID_WIDTH  640
#define VID_HEIGHT 400
#define VID_DUR    10
#define VID_FRATE  30
#define AUD_SRATE  44100

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
    if ((ret = frame_pusher_open(&pusher, argv[1], AUD_SRATE, VID_FRATE, VID_WIDTH, VID_HEIGHT, 800000)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot open output\n");
        return ret;
    }

    uint8_t *picture;
    // The buffer size needs to be multiplied by 3 because the format is RGB24
    int linesize = VID_WIDTH * 3 * sizeof(uint8_t);
    int bufsize = VID_HEIGHT * linesize;
    picture = (uint8_t *)av_malloc(bufsize);

    // Generate frames
    int i, j;
    uint8_t grey;
    float loudness;
    for (i = 0; i < VID_FRATE * VID_DUR; ++i) {
        // Video
        for (j = linesize * i; j < linesize * (i + 1); ++j)
            picture[j] = rand() % 256;
        if (i >= VID_FRATE * VID_DUR / 2) for (j = 0; j < 2333; ++j)
            picture[linesize * (rand() % i) + rand() % linesize] = rand() % 256;
        memset(picture + linesize * (i + 1), i < 150 ? 255 : 0, linesize);
        grey = (int)((double)i / (double)(VID_FRATE * VID_DUR - 1) * 255.0);
        memset(picture + linesize * (i + 2), grey, linesize * (VID_HEIGHT - i - 2));
        frame_pusher_write_video(pusher, picture, linesize, 1);
        // Audio
        loudness = ((1 + sin((float)i / 75.0 * M_PI)) / 2);
        loudness = loudness * loudness * 2 + 0.2;
        // For how to generate sine waves:
        // http://dsp.stackexchange.com/questions/14221
        double time;
        int16_t signal;
        for (j = 0; j < AUD_SRATE / VID_FRATE; ++j) {
            time = (double)(i * AUD_SRATE / VID_FRATE + j) / (double)AUD_SRATE;
            // Play a Major chord. Change the 4.0 to 3.0 to listen to a minor chord :)
            signal = (int16_t)(3000 * sin(1000 * M_PI * time));
            if (time >= 1) signal += (int16_t)(3000 * sin(1000 * pow(2, 4.0 / 12.0) * M_PI * time));
            if (time >= 2) signal += (int16_t)(3000 * sin(1000 * pow(2, 7.0 / 12.0) * M_PI * time));
            if (time >= 3) signal += (int16_t)(3000 * sin(1000 * 2 * M_PI * time));
            if ((5.05 <= time && time < 5.5) || (6.05 <= time && time < 6.5) || (7.05 <= time && time < 7.5)) signal = 0;
            // Attack & decay of 0.05 s...
            else if (5.0 <= time && time < 5.05) signal *= (5.05 - time) / 0.05;
            else if (5.5 <= time && time < 5.55) signal *= (time - 5.5) / 0.05;
            else if (6.0 <= time && time < 6.05) signal *= (6.05 - time) / 0.05;
            else if (6.5 <= time && time < 6.55) signal *= (time - 6.5) / 0.05;
            else if (7.0 <= time && time < 7.05) signal *= (7.05 - time) / 0.05;
            else if (7.5 <= time && time < 7.55) signal *= (time - 7.5) / 0.05;
            frame_pusher_write_audio(pusher, signal, signal);
        }
    }

    // Release resources
    frame_pusher_close(pusher);
    return 0;
}
