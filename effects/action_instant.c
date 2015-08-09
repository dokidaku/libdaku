#include "actions.h"
#include <stdlib.h>

daku_action *daku_fx_instant(daku_action_update_func update)
{
    daku_action *ret = (daku_action *)malloc(sizeof(daku_action));
    ret->duration = 0;
    ret->initialized = 1;
    ret->finalized = 0;
    ret->is_clip = 0;
    ret->init = NULL;
    ret->update = update;
    return ret;
}

void _daku_fx_flip_x_update(daku_action *action, float progress)
{
    action->target->flipped_x = !action->target->flipped_x;
}
daku_action *daku_fx_flip_x()
{
    return daku_fx_instant(&_daku_fx_flip_x_update);
}
void _daku_fx_flip_y_update(daku_action *action, float progress)
{
    action->target->flipped_y = !action->target->flipped_y;
}
daku_action *daku_fx_flip_y()
{
    return daku_fx_instant(&_daku_fx_flip_y_update);
}

void _daku_fx_freeze_update(daku_action *action, float progress)
{
    action->target->is_frozen = 1;
}
daku_action *daku_fx_freeze()
{
    return daku_fx_instant(&_daku_fx_freeze_update);
}
void _daku_fx_thaw_update(daku_action *action, float progress)
{
    action->target->is_frozen = 0;
}
daku_action *daku_fx_thaw()
{
    return daku_fx_instant(&_daku_fx_thaw_update);
}

void _daku_fx_setpixopacity_update(daku_action *action, float progress)
{
    daku_matter *target = action->target;
    uint16_t opacity = ((struct __daku_fx_setpixopacity *)action)->opacity;
    int x, y;
    for (x = 0; x < target->pict_width; ++x)
        for (y = 0; y < target->pict_height; ++y)
            target->picture[y * target->pict_width * 4 + x * 4 + 3] = opacity;
}
daku_action *daku_fx_setpixopacity(uint8_t opacity)
{
    struct __daku_fx_setpixopacity *ret =
        (struct __daku_fx_setpixopacity *)daku_fx_instant(&_daku_fx_setpixopacity_update);
    ret = realloc(ret, sizeof(struct __daku_fx_setpixopacity));
    ret->opacity = (float)opacity / 255.0 * 65535.0;
    return (daku_action *)ret;
}
daku_action *daku_fx_setpixopacitycont(float duration, uint8_t opacity)
{
    daku_action *ret = daku_fx_setpixopacity(opacity);
    ret->duration = duration;
    return ret;
}
