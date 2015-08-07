#include "ease.h"
#include <math.h>
#include <stdlib.h>

struct __daku_ease {
    daku_action base;
    daku_action *inner;
    float arg[4];
};

int _daku_ease_init(daku_action *action)
{
    daku_action *inner = ((struct __daku_ease *)action)->inner;
    inner->target = action->target;
    if (inner->init) return inner->init(inner);
    else return 0;
}
struct __daku_ease *daku_ease(daku_action *inner, daku_action_update_func update, float arg[4])
{
    struct __daku_ease *ret = (struct __daku_ease *)malloc(sizeof(struct __daku_ease));
    ret->base.duration = inner->duration;
    ret->base.initialized = inner->initialized;
    ret->base.finalized = 0;
    ret->base.is_clip = inner->is_clip;
    ret->base.init = &_daku_ease_init;
    ret->base.update = update;
    ret->inner = inner;
    ret->arg[0] = arg[0];
    ret->arg[1] = arg[1];
    ret->arg[2] = arg[2];
    ret->arg[3] = arg[3];
    return ret;
}

#define DAKU_EASE_UPD_IMPL(__name, __func) \
    void _daku_ease_##__name##_update(daku_action *action, float progress) \
    { \
        struct __daku_ease *r = (struct __daku_ease *)action; \
        daku_action *inner = r->inner; \
        inner->update(inner, (__func)); \
    }
#define DAKU_EASE_IMPL_ARG0(__name, __func) \
    DAKU_EASE_UPD_IMPL(__name, __func) \
    daku_action *daku_ease_##__name(daku_action *inner) \
    { \
        float arg[4] = {0, 0, 0, 0}; \
        return (daku_action *)daku_ease(inner, &_daku_ease_##__name##_update, arg); \
    }
#define DAKU_EASE_IMPL_ARG1(__name, __func, __init) \
    DAKU_EASE_UPD_IMPL(__name, __func) \
    daku_action *daku_ease_##__name(daku_action *inner, float arg0) \
    { \
        float arg[4] = {arg0, 0, 0, 0}; \
        struct __daku_ease *ret = daku_ease(inner, &_daku_ease_##__name##_update, arg); \
        __init; \
        return (daku_action *)ret; \
    }

DAKU_EASE_IMPL_ARG0(sine_in, 1 - sin(M_PI_2 * (1 - progress)));
DAKU_EASE_IMPL_ARG0(sine_out, sin(M_PI_2 * progress));
DAKU_EASE_IMPL_ARG0(sine_inout, (sin(M_PI_2 * (progress * 2 - 1)) + 1) * 0.5);

DAKU_EASE_IMPL_ARG0(quad_in, progress * progress);
DAKU_EASE_IMPL_ARG0(quad_out, 1 - (progress - 1) * (progress - 1));
DAKU_EASE_IMPL_ARG0(quad_inout, progress < 0.5 ? (progress * progress * 2) : (1 - (progress - 1) * (progress - 1) * 2));

DAKU_EASE_IMPL_ARG0(cubic_in, progress * progress * progress);
DAKU_EASE_IMPL_ARG0(cubic_out, 1 + (progress - 1) * (progress - 1) * (progress - 1));
DAKU_EASE_IMPL_ARG0(cubic_inout, progress < 0.5 ? (progress * progress * progress * 4) : (1 + (progress - 1) * (progress - 1) * (progress - 1) * 4));

DAKU_EASE_IMPL_ARG0(quart_in, progress * progress * progress * progress);
DAKU_EASE_IMPL_ARG0(quart_out, 1 - (progress - 1) * (progress - 1) * (progress - 1) * (progress - 1));
DAKU_EASE_IMPL_ARG0(quart_inout, progress < 0.5 ?
    (progress * progress * progress * progress * 8) : (1 - (progress - 1) * (progress - 1) * (progress - 1) * (progress - 1) * 8));

DAKU_EASE_IMPL_ARG0(quint_in, progress * progress * progress * progress * progress);
DAKU_EASE_IMPL_ARG0(quint_out, 1 + (progress - 1) * (progress - 1) * (progress - 1) * (progress - 1) * (progress - 1));
DAKU_EASE_IMPL_ARG0(quint_inout, progress < 0.5 ?
    (progress * progress * progress * progress * progress * 16) :
    (1 + (progress - 1) * (progress - 1) * (progress - 1) * (progress - 1) * (progress - 1) * 16));

DAKU_EASE_IMPL_ARG0(circ_in, 1 - sqrt(1 - progress * progress));
DAKU_EASE_IMPL_ARG0(circ_out, sqrt(1 - (progress - 1) * (progress - 1)));
DAKU_EASE_IMPL_ARG0(circ_inout, progress < 0.5 ?
    (0.5 - sqrt(0.25 - progress * progress)) :
    (0.5 + sqrt(0.25 - (progress - 1) * (progress - 1))));

DAKU_EASE_IMPL_ARG1(time_scale, progress, ret->base.duration *= arg[0]);
