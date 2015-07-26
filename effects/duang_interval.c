#include "duang.h"
#include <stdlib.h>

struct __daku_fx_move {
    daku_action base;
    float last_progress;
    float x, y;
};

void _daku_fx_moveby_update(daku_action *action, float progress)
{
    struct __daku_fx_move *duang = (struct __daku_fx_move *)action;
    action->target->x += (progress - duang->last_progress) * duang->x;
    action->target->y += (progress - duang->last_progress) * duang->y;
    duang->last_progress = progress;
}

daku_action *daku_fx_moveby(float duration, float dx, float dy)
{
    struct __daku_fx_move *ret =
        (struct __daku_fx_move *)malloc(sizeof(struct __daku_fx_move));
    ret->base.duration = duration;
    ret->base.update = &_daku_fx_moveby_update;
    ret->last_progress = 0;
    ret->x = dx;
    ret->y = dy;
    return (daku_action *)ret;
}

struct __daku_fx_fade {
    daku_action base;
    uint16_t start_opacity, end_opacity;
};

void _daku_fx_fadeto_update(daku_action *action, float progress)
{
    struct __daku_fx_fade *duang = (struct __daku_fx_fade *)action;
    action->target->opacity = (uint16_t)((float)duang->start_opacity * (1 - progress) + (float)duang->end_opacity * progress);
}

daku_action *daku_fx_fadeto(float duration, uint8_t opacity)
{
    struct __daku_fx_fade *ret =
        (struct __daku_fx_fade *)malloc(sizeof(struct __daku_fx_fade));
    ret->base.duration = duration;
    ret->base.update = &_daku_fx_fadeto_update;
    ret->start_opacity = 65535;
    ret->end_opacity = opacity << 8;
    return (daku_action *)ret;
}
