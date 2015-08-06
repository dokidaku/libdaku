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
    return inner->init(inner);
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

void _daku_ease_sine_in_update(daku_action *action, float progress)
{
    daku_action *inner = ((struct __daku_ease *)action)->inner;
    inner->update(inner, 1 - sin(M_PI_2 * (1 - progress)));
}
daku_action *daku_ease_sine_in(daku_action *inner)
{
    return daku_ease(inner, &_daku_ease_sine_in_update);
}
