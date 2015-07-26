// Example for frame_puller and frame_pusher: video converting / transcoding
// $ gcc -c ../frame-puller.c -o frame-puller.o
// $ gcc -c ../frame-pusher.c -o frame-pusher.o
// $ gcc convert.c frame-puller.o frame-pusher.o -o convert `pkg-config --cflags --libs libavformat libswscale libswresample`

#include "../frame-puller.h"
#include "../frame-pusher.h"
#include <libswscale/swscale.h>

int main(int argc, char *argv[])
{
    if (argc < 3) {
        printf("Video converting based on frame pullers & pushers\n");
        printf("Usage: %s <input> <output> [<start_time> [<frame_rate> [<bit_rate>]]]\n", argv[0]);
        printf("Reads a video and outputs it to another format.\n");
        return 1;
    }
    av_register_all();

    // Parse arguments
    float start_time = 0;
    int frame_rate = 0, bit_rate = 0;
    if (argc >= 4) start_time = atof(argv[3]);
    if (argc >= 5) frame_rate = atoi(argv[4]);
    if (argc >= 6) bit_rate = atoi(argv[5]);
    if (frame_rate <= 0) frame_rate = 30;
    if (bit_rate < 0) bit_rate = 0; // Let the frame_pusher automatically fill this

    int ret;
    frame_puller *puller_a, *puller_v;
    if ((ret = frame_puller_open_video(&puller_v, argv[1], 0, 0, 0)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot open input file\n");
        return ret;
    }
    if ((ret = frame_puller_open_audio(&puller_a, argv[1])) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot read audio from input file\n");
        return ret;
    }
    frame_pusher *pusher;
    if ((ret = frame_pusher_open(&pusher, argv[2], puller_a->codec_ctx->sample_rate, frame_rate,
        puller_v->output_width, puller_v->output_height, bit_rate)) < 0)
    {
        av_log(NULL, AV_LOG_ERROR, "Cannot initialize the output file\n");
        return ret;
    }

    AVFrame *frame;

    int nb_frames_read = 0, nb_samples_read = 0;
    int i;
    unsigned char has_video = 1, has_audio = 1, catches;
    if (start_time) {
        frame_puller_seek(puller_a, start_time);
        frame_puller_seek(puller_v, start_time);
    }
    // Continuously get frames and convert.
    // TODO: Interleave video frames and audio frames with the help of av_compare_ts.
    do {
        catches = frame_pusher_audio_catches_audio(pusher);
        if (has_video && (catches || !has_audio)) {
            if (frame_puller_next_frame(puller_a, &frame) >= 0) {
                nb_samples_read += frame->nb_samples;
                for (i = 0; i < frame->nb_samples; ++i)
                    if ((ret = frame_pusher_write_audio(pusher,
                        *(int16_t *)&frame->data[0][i + i + 1],
                        *(int16_t *)&frame->data[puller_a->codec_ctx->channels >= 2 ? 1 : 0][i + i + 1])) < 0)
                    {
                        av_log(NULL, AV_LOG_ERROR, "Failed to write one of the samples. Quitting T^T\n");
                        return ret;
                    }
            } else has_video = 0;
        } else if (has_audio && (!catches || !has_video)) {
            if (frame_puller_next_frame(puller_v, &frame) >= 0) {
                ++nb_frames_read;
                // Save to output.
                if ((ret = frame_pusher_write_video(pusher, frame->data, frame->linesize, 1)) < 0) {
                    av_log(NULL, AV_LOG_ERROR, "Failed to write frame #%d. Quitting T^T\n", nb_frames_read);
                    return ret;
                }
            } else has_audio = 0;
        }
    } while (has_video || has_audio);
    printf("Total video frames: %d\nTotal audio samples: %d\n", nb_frames_read, nb_samples_read);

    // Release resources
    frame_puller_free(puller_v);
    frame_puller_free(puller_a);
    frame_pusher_close(pusher);
    return 0;
}
