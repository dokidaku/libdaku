#include "duang.h"
#include <stdlib.h>

daku_action *daku_fx_instant(daku_action_update_func update)
{
    daku_action *ret = (daku_action *)malloc(sizeof(daku_action));
    ret->duration = 0;
    ret->initialized = 1;
    ret->finalized = 0;
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
