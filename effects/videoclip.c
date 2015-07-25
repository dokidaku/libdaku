#include "videoclip.h"
#include <libavformat/avformat.h>
#include "../utils/frame-puller.h"

struct __daku_video_clip {
    daku_action base;
    frame_puller *puller;
    float start_time;
    double time_base;
    int vid_width, vid_height;
};

void _daku_video_clip_update(daku_action *action, float progress)
{
    struct __daku_video_clip *duang = (struct __daku_video_clip *)action;
    int x, y;
    while (duang->puller->packet.pts < (duang->start_time + progress * action->duration) * duang->time_base) {
        frame_puller_next_frame(duang->puller, NULL);
    }
    AVFrame *f = duang->puller->frame;
    for (y = 0; y < duang->vid_height; ++y)
        for (x = 0; x < duang->vid_width; ++x) {
            action->target->picture[(y * duang->vid_width + x) * 4 + 0] = f->data[0][y * f->linesize[0] + x * 3 + 0] << 8;
            action->target->picture[(y * duang->vid_width + x) * 4 + 1] = f->data[0][y * f->linesize[0] + x * 3 + 1] << 8;
            action->target->picture[(y * duang->vid_width + x) * 4 + 2] = f->data[0][y * f->linesize[0] + x * 3 + 2] << 8;
            action->target->picture[(y * duang->vid_width + x) * 4 + 3] = 65535;
        }
}

daku_action *daku_video_clip(const char *path, float start_time, float duration, int width, int height)
{
    struct __daku_video_clip *ret =
        (struct __daku_video_clip *)malloc(sizeof(struct __daku_video_clip));
    ret->base.duration = duration;
    ret->base.update = &_daku_video_clip_update;
    if (frame_puller_open_video(&ret->puller, path, width, height) < 0) return NULL;
    if (frame_puller_seek(ret->puller, start_time) < 0) return NULL;
    AVRational *tb = &ret->puller->fmt_ctx->streams[ret->puller->target_stream_idx]->time_base;
    ret->start_time = start_time;
    ret->time_base = (double)tb->den / (double)tb->num;
    ret->vid_width = ret->puller->output_width;
    ret->vid_height = ret->puller->output_height;
    return (daku_action *)ret;
}
