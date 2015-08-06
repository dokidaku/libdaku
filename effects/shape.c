#include "matters.h"
#include <math.h>
#include <stdlib.h>

struct __daku_matter_shape {
    daku_action base;
    enum daku_shape_type type;
    uint16_t r, g, b, a;
};

void _daku_matter_shape_update(daku_action *action, float progress)
{
    int i, j, w = action->target->pict_width, h = action->target->pict_height;
    float cur_y, x_range, x_start, x_end;
    int r = ((struct __daku_matter_shape *)action)->r,
        g = ((struct __daku_matter_shape *)action)->g,
        b = ((struct __daku_matter_shape *)action)->b,
        a = ((struct __daku_matter_shape *)action)->a;
    switch (((struct __daku_matter_shape *)action)->type) {
        case DAKU_SHAPE_RECT:
            for (i = 0; i < h; ++i)
                for (j = 0; j < w; ++j) {
                    action->target->picture[i * w * 4 + j * 4] = r;
                    action->target->picture[i * w * 4 + j * 4 + 1] = g;
                    action->target->picture[i * w * 4 + j * 4 + 2] = b;
                    action->target->picture[i * w * 4 + j * 4 + 3] = a;
                }
            break;
        case DAKU_SHAPE_ELLIPSE:
            // XXX: Do we need anti-aliasing??
            for (i = 0; i < h; ++i) {
                cur_y = 0.5 - (i + 0.5) / (float)h;
                x_range = sqrt(0.25 - cur_y * cur_y);
                x_start = ceil((0.5 - x_range) * w);
                x_end = trunc((0.5 + x_range) * w);
                for (j = x_start; j <= x_end; ++j) {
                    action->target->picture[i * w * 4 + j * 4] = r;
                    action->target->picture[i * w * 4 + j * 4 + 1] = g;
                    action->target->picture[i * w * 4 + j * 4 + 2] = b;
                    action->target->picture[i * w * 4 + j * 4 + 3] = a;
                }
            }
        default: break;
    }
}

daku_action *daku_matter_shape(float duration, enum daku_shape_type type, int colour, int opacity)
{
    struct __daku_matter_shape *ret =
        (struct __daku_matter_shape *)malloc(sizeof(struct __daku_matter_shape));
    ret->base.duration = duration;
    ret->base.initialized = 0;
    ret->base.is_clip = 1;
    ret->base.update = _daku_matter_shape_update;
    ret->type = type;
    ret->r = (colour & 0xff0000) >> 8;
    ret->g = colour & 0x00ff00;
    ret->b = (colour & 0x0000ff) << 8;
    ret->a = opacity << 8;
    return (daku_action *)ret;
}
