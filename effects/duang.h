#ifndef __DAKU_DUANG_H__
#define __DAKU_DUANG_H__

#include "../types.h"

// duang_interval.c
daku_action *daku_fx_moveby(float duration, float dx, float dy);    // d stands for delta
daku_action *daku_fx_moveto(float duration, float dx, float dy);    // d stands for destination

daku_action *daku_fx_fadeto(float duration, uint8_t opacity);
daku_action *daku_fx_fadein(float duration);
daku_action *daku_fx_fadeout(float duration);

daku_action *daku_fx_scaleto(float duration, float scale);

daku_action *daku_fx_rotateto(float duration, float angle_deg);

daku_action *daku_fx_skewby(float duration, float x_angle, float y_angle);

// duang_audio.c
daku_instrument *daku_audio_reverse(float duration);

#endif
