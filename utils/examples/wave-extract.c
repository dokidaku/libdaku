// Example for frame_puller: waveform extractor
// $ gcc -c ../frame-puller.c -o frame-puller.o
// $ gcc wave-extract.c frame-puller.o -o wave-extract `pkg-config --cflags --libs libavformat libswscale libswresample`

#include "../frame-puller.h"

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("Waveform extractor based on frame pullers\n");
        printf("Usage: %s <input>\n", argv[0]);
        printf("Extracts waveform data from a video or audio.\n");
        printf("The amplitude data will be printed to standard output.\n");
        printf("The output may be **very** large so redirecting output to a file is recommended.\n");
        printf("After that, a plotting program can be used to generate waveform graphs from numbers.\n");
        return 1;
    }
    av_register_all();

    int ret;
    frame_puller *fp;
    if ((ret = frame_puller_open_audio(&fp, argv[1])) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot open input file\n");
        return ret;
    }

    AVFrame *frame;
    int nb_frames_read = 0, nb_samples_read = 0, i;
    puts("Stereo");
    puts("L-chan | R-chan");
    // Continuously get audio frames and output data.
    while (frame_puller_next_frame(fp, &frame) >= 0) {
        ++nb_frames_read;
        nb_samples_read += frame->nb_samples;
        for (i = 0; i < frame->nb_samples; ++i)
            printf("%d %d\n", *(int16_t *)&frame->data[0][i + i + 1], *(int16_t *)&frame->data[1][i + i + 1]);
    }
    printf("Total frames: %d\nTotal samples: %d\n", nb_frames_read, nb_samples_read);

    // Release resources
    frame_puller_free(fp);
    return 0;
}
