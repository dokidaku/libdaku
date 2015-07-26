#ifndef __DAKU_DUANG_H__
#define __DAKU_DUANG_H__

#include "../types.h"

// duang_interval.c
daku_action *daku_fx_moveby(float duration, float dx, float dy);

daku_action *daku_fx_fadeto(float duration, uint8_t opacity);

#endif
