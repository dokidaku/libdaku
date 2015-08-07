#include "actions.h"
#include <math.h>
#include <stdlib.h>

#ifndef MAX
#define MAX(__a, __b) ((__a) > (__b) ? (__a) : (__b))
#endif
#ifndef MIN
#define MIN(__a, __b) ((__a) < (__b) ? (__a) : (__b))
#endif

struct __daku_transition {
    daku_action base;
    uint16_t opacity;
    float last_progress;
};

void _daku_tr_erase_l_update(daku_action *action, float progress)
{
    daku_matter *target = action->target;
    float last_progress = ((struct __daku_transition *)action)->last_progress;
    uint16_t opacity = ((struct __daku_transition *)action)->opacity;
    int x, y;
    for (x = last_progress * target->pict_width; x < progress * target->pict_width; ++x)
        for (y = 0; y < target->pict_height; ++y)
            target->picture[y * target->pict_width * 4 + x * 4 + 3] = opacity;
    ((struct __daku_transition *)action)->last_progress = progress;
}
daku_action *daku_tr_erase_l(float duration, uint8_t opacity)
{
    struct __daku_transition *ret =
        (struct __daku_transition *)malloc(sizeof(struct __daku_transition));
    ret->base.duration = duration;
    ret->base.initialized = 0;
    ret->base.is_clip = 1;
    ret->base.init = NULL;
    ret->base.update = &_daku_tr_erase_l_update;
    ret->opacity = opacity << 8;
    ret->last_progress = 0;
    return (daku_action *)ret;
}

void _daku_tr_erase_circle_update(daku_action *action, float progress)
{
    daku_matter *target = action->target;
    uint16_t opacity = ((struct __daku_transition *)action)->opacity;
    float radius = sqrt(target->pict_width * target->pict_width + target->pict_height * target->pict_height) * 0.5 * progress;
    int x, y, x1, x2, y1, y2;
    float yy, rr;
    y1 = MAX(0, target->pict_height / 2 - radius);
    y2 = MIN(target->pict_height - 1, target->pict_height / 2 + radius);
    for (y = y1; y <= y2; ++y) {
        yy = y - target->pict_height / 2;
        rr = sqrt(radius * radius - yy * yy);
        x1 = MAX(0, target->pict_width / 2 - rr);
        x2 = MIN(target->pict_width - 1, target->pict_width / 2 + rr);
        for (x = x1; x <= x2; ++x)
            target->picture[y * target->pict_width * 4 + x * 4 + 3] = opacity;
    }
    ((struct __daku_transition *)action)->last_progress = progress;
}
daku_action *daku_tr_erase_circle(float duration, uint8_t opacity)
{
    struct __daku_transition *ret =
        (struct __daku_transition *)malloc(sizeof(struct __daku_transition));
    ret->base.duration = duration;
    ret->base.initialized = 0;
    ret->base.is_clip = 1;
    ret->base.init = NULL;
    ret->base.update = &_daku_tr_erase_circle_update;
    ret->opacity = opacity << 8;
    ret->last_progress = 0;
    return (daku_action *)ret;
}

#undef MAX
#undef MIN
