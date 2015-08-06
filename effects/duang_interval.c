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
    ret->base.initialized = ret->base.finalized = 0;
    ret->base.init = NULL;
    ret->base.update = &_daku_fx_moveby_update;
    ret->last_progress = 0;
    ret->x = dx;
    ret->y = dy;
    return (daku_action *)ret;
}
int _daku_fx_moveto_init(daku_action *action)
{
    struct __daku_fx_move *duang = (struct __daku_fx_move *)action;
    duang->x -= action->target->x;
    duang->y -= action->target->y;
    return 0;
}
daku_action *daku_fx_moveto(float duration, float dx, float dy)
{
    daku_action *ret = daku_fx_moveby(duration, dx, dy);
    ret->init = &_daku_fx_moveto_init;
    return ret;
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
int _daku_fx_fadeto_init(daku_action *action)
{
    struct __daku_fx_fade *ret = (struct __daku_fx_fade *)action;
    ret->start_opacity = action->target->opacity;
    return 0;
}
daku_action *daku_fx_fadeto(float duration, uint8_t opacity)
{
    struct __daku_fx_fade *ret =
        (struct __daku_fx_fade *)malloc(sizeof(struct __daku_fx_fade));
    ret->base.duration = duration;
    ret->base.initialized = ret->base.finalized = 0;
    ret->base.init = &_daku_fx_fadeto_init;
    ret->base.update = &_daku_fx_fadeto_update;
    ret->end_opacity = opacity << 8;
    return (daku_action *)ret;
}
daku_action *daku_fx_fadein(float duration)
{
    return daku_fx_fadeto(duration, 255);
}
daku_action *daku_fx_fadeout(float duration)
{
    return daku_fx_fadeto(duration, 0);
}

struct __daku_fx_scale {
    daku_action base;
    float start_scale_x, end_scale_x;
    float start_scale_y, end_scale_y;
};
void _daku_fx_scaleto_update(daku_action *action, float progress)
{
    struct __daku_fx_scale *duang = (struct __daku_fx_scale *)action;
    action->target->scale_x = duang->start_scale_x * (1 - progress) + duang->end_scale_x * progress;
    action->target->scale_y = duang->start_scale_y * (1 - progress) + duang->end_scale_y * progress;
}
int _daku_fx_scaleto_init(daku_action *action)
{
    struct __daku_fx_scale *ret = (struct __daku_fx_scale *)action;
    ret->start_scale_x = action->target->scale_x;
    ret->start_scale_y = action->target->scale_y;
    return 0;
}
daku_action *daku_fx_scaleto(float duration, float scale_x, float scale_y)
{
    struct __daku_fx_scale *ret =
        (struct __daku_fx_scale *)malloc(sizeof(struct __daku_fx_scale));
    ret->base.duration = duration;
    ret->base.initialized = ret->base.finalized = 0;
    ret->base.init = &_daku_fx_scaleto_init;
    ret->base.update = &_daku_fx_scaleto_update;
    ret->end_scale_x = scale_x;
    ret->end_scale_y = scale_y;
    return (daku_action *)ret;
}
int _daku_fx_scaleby_init(daku_action *action)
{
    struct __daku_fx_scale *ret = (struct __daku_fx_scale *)action;
    ret->start_scale_x = action->target->scale_x;
    ret->start_scale_y = action->target->scale_y;
    ret->end_scale_x *= action->target->scale_x;
    ret->end_scale_y *= action->target->scale_y;
    return 0;
}
daku_action *daku_fx_scaleby(float duration, float scale_x, float scale_y)
{
    daku_action *ret = daku_fx_scaleto(duration, scale_x, scale_y);
    ret->init = &_daku_fx_scaleby_init;
    return ret;
}

struct __daku_fx_rotate {
    daku_action base;
    float start_angle, end_angle;
};
void _daku_fx_rotateto_update(daku_action *action, float progress)
{
    struct __daku_fx_rotate *duang = (struct __daku_fx_rotate *)action;
    action->target->rotation = duang->start_angle * (1 - progress) + duang->end_angle * progress;
}
int _daku_fx_rotateto_init(daku_action *action)
{
    struct __daku_fx_rotate *ret = (struct __daku_fx_rotate *)action;
    ret->start_angle = action->target->rotation;
    return 0;
}
daku_action *daku_fx_rotateto(float duration, float angle_deg)
{
    struct __daku_fx_rotate *ret =
        (struct __daku_fx_rotate *)malloc(sizeof(struct __daku_fx_rotate));
    ret->base.duration = duration;
    ret->base.initialized = ret->base.finalized = 0;
    ret->base.init = &_daku_fx_rotateto_init;
    ret->base.update = &_daku_fx_rotateto_update;
    ret->end_angle = angle_deg;
    return (daku_action *)ret;
}
int _daku_fx_rotateby_init(daku_action *action)
{
    struct __daku_fx_rotate *ret = (struct __daku_fx_rotate *)action;
    ret->start_angle = action->target->rotation;
    ret->end_angle += action->target->rotation;
    return 0;
}
daku_action *daku_fx_rotateby(float duration, float angle_deg)
{
    daku_action *ret = (daku_action *)daku_fx_rotateto(duration, angle_deg);
    ret->init = &_daku_fx_rotateby_init;
    return ret;
}

typedef struct __daku_fx_move __daku_fx_skew;   // (/_<)
void _daku_fx_skewby_update(daku_action *action, float progress)
{
    __daku_fx_skew *duang = (__daku_fx_skew *)action;
    action->target->skew_x += (progress - duang->last_progress) * duang->x;
    action->target->skew_y += (progress - duang->last_progress) * duang->y;
    duang->last_progress = progress;
}
daku_action *daku_fx_skewby(float duration, float x_angle, float y_angle)
{
    __daku_fx_skew *ret = (__daku_fx_skew *)malloc(sizeof(__daku_fx_skew));
    ret->base.duration = duration;
    ret->base.initialized = ret->base.finalized = 0;
    ret->base.init = NULL;
    ret->base.update = &_daku_fx_skewby_update;
    ret->last_progress = 0;
    ret->x = x_angle;
    ret->y = y_angle;
    return (daku_action *)ret;
}
int _daku_fx_skewto_init(daku_action *action)
{
    __daku_fx_skew *ret = (__daku_fx_skew *)action;
    ret->x -= action->target->skew_x;
    ret->y -= action->target->skew_y;
    return 0;
}
daku_action *daku_fx_skewto(float duration, float x_angle, float y_angle)
{
    daku_action *ret = daku_fx_skewby(duration, x_angle, y_angle);
    ret->init = &_daku_fx_skewto_init;
    return ret;
}
