#include "matters.h"
#include <stdlib.h>

struct __daku_matter_shape {
    daku_action base;
    enum daku_shape_type type;
    uint8_t r, g, b;
};

void _daku_matter_shape_update(daku_action *action, float progress)
{
    int i, j, w = action->target->pict_width, h = action->target->pict_height;
    int r = ((struct __daku_matter_shape *)action)->r,
        g = ((struct __daku_matter_shape *)action)->g,
        b = ((struct __daku_matter_shape *)action)->b;
    switch (((struct __daku_matter_shape *)action)->type) {
        case DAKU_SHAPE_RECT:
            for (i = 0; i < h; ++i)
                for (j = 0; j < w; ++j) {
                    action->target->picture[i * w * 3 + j * 3] = r;
                    action->target->picture[i * w * 3 + j * 3 + 1] = g;
                    action->target->picture[i * w * 3 + j * 3 + 2] = b;
                }
            break;
        default: break;
    }
}

daku_action *daku_matter_shape(float duration, enum daku_shape_type type, int colour)
{
    struct __daku_matter_shape *ret =
        (struct __daku_matter_shape *)malloc(sizeof(struct __daku_matter_shape));
    ret->base.duration = duration;
    ret->base.update = _daku_matter_shape_update;
    ret->type = type;
    ret->r = colour & 0xff0000;
    ret->g = colour & 0x00ff00;
    ret->b = colour & 0x0000ff;
    return (daku_action *)ret;
}
