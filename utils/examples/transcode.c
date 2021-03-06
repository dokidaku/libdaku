// Example for frame_puller and frame_pusher: video converting / transcoding
// $ gcc transcode.c ../frame-puller.c ../frame-pusher.c -o transcode `pkg-config --cflags --libs libavformat libswscale libswresample`

#include <time.h>
#include "../frame-puller.h"
#include "../frame-pusher.h"

int main(int argc, char *argv[])
{
    int prog_start_time = time(0);
    if (argc < 3) {
        printf("Video converting based on frame pullers & pushers\n");
        printf("Usage: %s <input> <output> [<start_time> [<frame_rate> [<sample_rate> [<bit_rate>]]]]\n", argv[0]);
        printf("Reads a video and outputs it to another format.\n");
        return 1;
    }
    av_register_all();

    // Parse arguments
    float start_time = 0;
    int frame_rate = 0, sample_rate = 0, bit_rate = 0;
    if (argc >= 4) start_time = atof(argv[3]);
    if (argc >= 5) frame_rate = atoi(argv[4]);
    if (argc >= 6) sample_rate = atoi(argv[5]);
    if (argc >= 7) bit_rate = atoi(argv[6]);
    if (frame_rate <= 0) frame_rate = 30;
    if (bit_rate < 0) bit_rate = 0; // Let the frame_pusher automatically fill this

    int ret;
    frame_puller *puller_a, *puller_v;
    if ((ret = frame_puller_open_video(&puller_v, argv[1], 0, 0, PIX_FMT_RGB24, 0)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot open input file\n");
        return ret;
    }
    if ((ret = frame_puller_open_audio(&puller_a, argv[1], sample_rate)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot read audio from input file\n");
        return ret;
    }
    frame_pusher *pusher;
    if ((ret = frame_pusher_open(&pusher, argv[2],
        puller_a->output_sample_rate, (AVRational){frame_rate, 1},
        puller_v->output_width, puller_v->output_height, bit_rate)) < 0)
    {
        av_log(NULL, AV_LOG_ERROR, "Cannot initialize the output file\n");
        return ret;
    }

    AVFrame *aud_frame, *vid_frame;

    int nb_frames_read = 0, nb_frames_written = 0, nb_samples_read = 0;
    int i;
    unsigned char has_video = 1, has_audio = 1, catches;
    int16_t *lch_ptr, *rch_ptr;
    if (start_time) {
        frame_puller_seek(puller_a, start_time, 1);
        frame_puller_seek(puller_v, start_time, 1);
    }
    // Continuously get frames and convert.
    puller_v->frame->pts = -233;    // Force a frame read at the beginning
    do {
        catches = frame_pusher_audio_catches_audio(pusher);
        if (has_video && (catches || !has_audio)) {
            if (frame_puller_next_frame(puller_a, &aud_frame) >= 0) {
                nb_samples_read += aud_frame->nb_samples;
                lch_ptr = (int16_t *)(aud_frame->data[0]);
                rch_ptr = (int16_t *)(aud_frame->data[puller_a->codec_ctx->channels >= 2 ? 1 : 0]);
                for (i = 0; i < aud_frame->nb_samples; ++i) {
                    if ((ret = frame_pusher_write_audio(pusher, lch_ptr[i], rch_ptr[i])) < 0) {
                        av_log(NULL, AV_LOG_ERROR, "Failed to write one of the samples. Quitting T^T\n");
                        return ret;
                    }
                }
            } else has_video = 0;
        } else if (has_audio && (!catches || !has_video)) {
            // TODO: Handle cases when the output framerate is lower than input (just changing the 'if' to 'while' will work)
            if (frame_puller_last_time(puller_v) <= start_time + (float)nb_frames_written / frame_rate) {
                if (frame_puller_next_frame(puller_v, &vid_frame) >= 0) {
                    ++nb_frames_read;
                } else {
                    has_audio = 0;
                    break;
                }
            }
            // Save to output.
            ++nb_frames_written;
            if ((ret = frame_pusher_write_video(pusher, vid_frame->data[0], vid_frame->linesize[0], 1)) < 0) {
                av_log(NULL, AV_LOG_ERROR, "Failed to write frame #%d. Quitting T^T\n", nb_frames_read);
                return ret;
            }
            if (nb_frames_read >= 500 || nb_frames_written >= 500) break;
        }
    } while (has_video || has_audio);
    printf("Total video frames: %d\nTotal audio samples: %d\n", nb_frames_read, nb_samples_read);

    // Release resources
    frame_puller_free(puller_v);
    frame_puller_free(puller_a);
    frame_pusher_close(pusher);

    printf("User time: %.2f s\n", (double)clock() / CLOCKS_PER_SEC);
    printf("Real time: ~%ld s\n", time(0) - prog_start_time);
    return 0;
}
