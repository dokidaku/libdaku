#include "videoclip.h"
#include <libavformat/avformat.h>
#include "../utils/frame-puller.h"

struct __daku_video_clip {
    daku_action base;
    const char *path;
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
    uint16_t *frame_pict = (uint16_t *)f->data[0];
    int subscript_inc = f->linesize[0] / 2;
    for (y = 0; y < duang->vid_height; ++y)
        for (x = 0; x < duang->vid_width; ++x) {
            action->target->picture[(((duang->vid_height - y - 1) * duang->vid_width + x) << 2) + 0] = frame_pict[y * subscript_inc + x * 3 + 0];
            action->target->picture[(((duang->vid_height - y - 1) * duang->vid_width + x) << 2) + 1] = frame_pict[y * subscript_inc + x * 3 + 1];
            action->target->picture[(((duang->vid_height - y - 1) * duang->vid_width + x) << 2) + 2] = frame_pict[y * subscript_inc + x * 3 + 2];
            action->target->picture[(((duang->vid_height - y - 1) * duang->vid_width + x) << 2) + 3] = 65535;
        }
}
int _daku_video_clip_init(daku_action *action)
{
    struct __daku_video_clip *ret = (struct __daku_video_clip *)action;
    int width = action->target->pict_width, height = action->target->pict_height;
    if (frame_puller_open_video(&ret->puller, ret->path, width, height, 1) < 0) return -4;
    if (frame_puller_seek(ret->puller, ret->start_time) < 0) return -6;
    AVRational *tb = &ret->puller->fmt_ctx->streams[ret->puller->target_stream_idx]->time_base;
    ret->time_base = (double)tb->den / (double)tb->num;
    ret->vid_width = ret->puller->output_width;
    ret->vid_height = ret->puller->output_height;
}
daku_action *daku_video_clip(const char *path, float start_time, float duration)
{
    struct __daku_video_clip *ret =
        (struct __daku_video_clip *)malloc(sizeof(struct __daku_video_clip));
    ret->base.duration = duration;
    ret->base.init = &_daku_video_clip_init;
    ret->base.update = &_daku_video_clip_update;
    ret->path = path;
    ret->start_time = start_time;
    return (daku_action *)ret;
}
