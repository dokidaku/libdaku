#include "ease.h"
#include <math.h>
#include <stdlib.h>

struct __daku_ease {
    daku_action base;
    daku_action *inner;
};

int _daku_ease_init(daku_action *action)
{
    daku_action *inner = ((struct __daku_ease *)action)->inner;
    inner->target = action->target;
    if (inner->init) return inner->init(inner);
    else return 0;
}
daku_action *daku_ease(daku_action *inner, daku_action_update_func update)
{
    struct __daku_ease *ret = (struct __daku_ease *)malloc(sizeof(struct __daku_ease));
    ret->base.duration = inner->duration;
    ret->base.initialized = inner->initialized;
    ret->base.finalized = 0;
    ret->base.is_clip = inner->is_clip;
    ret->base.init = &_daku_ease_init;
    ret->base.update = update;
    ret->inner = inner;
    return (daku_action *)ret;
}

#define DAKU_EASE_IMPL(__name, __func) \
void _daku_ease_##__name##_update(daku_action *action, float progress) \
{ \
    daku_action *inner = ((struct __daku_ease *)action)->inner; \
    inner->update(inner, (__func)); \
} \
daku_action *daku_ease_##__name(daku_action *inner) \
{ \
    return daku_ease(inner, &_daku_ease_##__name##_update); \
}

DAKU_EASE_IMPL(sine_in, 1 - sin(M_PI_2 * (1 - progress)));
DAKU_EASE_IMPL(sine_out, sin(M_PI_2 * progress));
DAKU_EASE_IMPL(sine_inout, (sin(M_PI_2 * (progress * 2 - 1)) + 1) * 0.5);

DAKU_EASE_IMPL(quad_in, progress * progress);
DAKU_EASE_IMPL(quad_out, 1 - (progress - 1) * (progress - 1));
DAKU_EASE_IMPL(quad_inout, progress < 0.5 ? (progress * progress * 2) : (1 - (progress - 1) * (progress - 1) * 2));

DAKU_EASE_IMPL(cubic_in, progress * progress * progress);
DAKU_EASE_IMPL(cubic_out, 1 + (progress - 1) * (progress - 1) * (progress - 1));
DAKU_EASE_IMPL(cubic_inout, progress < 0.5 ? (progress * progress * progress * 4) : (1 + (progress - 1) * (progress - 1) * (progress - 1) * 4));

DAKU_EASE_IMPL(quart_in, progress * progress * progress * progress);
DAKU_EASE_IMPL(quart_out, 1 - (progress - 1) * (progress - 1) * (progress - 1) * (progress - 1));
DAKU_EASE_IMPL(quart_inout, progress < 0.5 ?
    (progress * progress * progress * progress * 8) : (1 - (progress - 1) * (progress - 1) * (progress - 1) * (progress - 1) * 8));

DAKU_EASE_IMPL(quint_in, progress * progress * progress * progress * progress);
DAKU_EASE_IMPL(quint_out, 1 + (progress - 1) * (progress - 1) * (progress - 1) * (progress - 1) * (progress - 1));
DAKU_EASE_IMPL(quint_inout, progress < 0.5 ?
    (progress * progress * progress * progress * progress * 16) :
    (1 + (progress - 1) * (progress - 1) * (progress - 1) * (progress - 1) * (progress - 1) * 16));

DAKU_EASE_IMPL(circ_in, 1 - sqrt(1 - progress * progress));
DAKU_EASE_IMPL(circ_out, sqrt(1 - (progress - 1) * (progress - 1)));
DAKU_EASE_IMPL(circ_inout, progress < 0.5 ?
    (0.5 - sqrt(0.25 - progress * progress)) :
    (0.5 + sqrt(0.25 - (progress - 1) * (progress - 1))));
