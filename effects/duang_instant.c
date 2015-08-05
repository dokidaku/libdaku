#include "duang.h"
#include <stdlib.h>

void _daku_fx_flip_x_update(daku_action *action, float progress)
{
    action->target->flipped_x = !action->target->flipped_x;
}
daku_action *daku_fx_flip_x()
{
    daku_action *ret = (daku_action *)malloc(sizeof(daku_action));
    ret->duration = 0;
    ret->initialized = 1;
    ret->finalized = 0;
    ret->init = NULL;
    ret->update = &_daku_fx_flip_x_update;
    return ret;
}
