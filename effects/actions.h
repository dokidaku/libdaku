#ifndef __DAKU_ACTIONS_H__
#define __DAKU_ACTIONS_H__

#include "../types.h"

// action_interval.c
daku_action *daku_fx_moveby(float duration, float dx, float dy);    // d stands for delta
daku_action *daku_fx_moveto(float duration, float dx, float dy);    // d stands for destination

daku_action *daku_fx_fadeto(float duration, uint8_t opacity);
daku_action *daku_fx_fadein(float duration);
daku_action *daku_fx_fadeout(float duration);

daku_action *daku_fx_scaleto(float duration, float scale_x, float scale_y);
daku_action *daku_fx_scaleby(float duration, float scale_x, float scale_y);
#define daku_fx_zoomto(__duration, __scale) daku_fx_scaleto((__duration), (__scale), (__scale))
#define daku_fx_zoomby(__duration, __scale) daku_fx_scaleby((__duration), (__scale), (__scale))

daku_action *daku_fx_rotateto(float duration, float angle_deg);
daku_action *daku_fx_rotateby(float duration, float angle_deg);

daku_action *daku_fx_skewto(float duration, float x_angle, float y_angle);
daku_action *daku_fx_skewby(float duration, float x_angle, float y_angle);

// action_instant.c
daku_action *daku_fx_flip_x();
daku_action *daku_fx_flip_y();

daku_action *daku_fx_freeze();
daku_action *daku_fx_thaw();

// audio.c
daku_instrument *daku_audio_reverse(float duration);

#endif
