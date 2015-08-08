#ifndef __DAKU_CORE_H__
#define __DAKU_CORE_H__

#include "types.h"

#ifdef __cplusplus
#include <algorithm>
#endif
// Video part
daku_matter *daku_matter_create();
void daku_matter_setlife(daku_matter *m, float life_time);
void daku_matter_setsize(daku_matter *m, float width, float height);
void daku_matter_setgrosssize(daku_matter *m, float nw, float nh, float gw, float gh);
void daku_matter_setanchor(daku_matter *m, float ax, float ay);
void daku_matter_setpos(daku_matter *m, float x, float y);
void daku_matter_setopacity(daku_matter *m, uint8_t opacity);
void daku_matter_setscale(daku_matter *m, float scale_x, float scale_y);
#define daku_matter_setzoom(__m, __s) daku_matter_setscale((__m), (__s), (__s))
void daku_matter_act(daku_matter *m, float start_time, daku_action *action);
void daku_matter_init(daku_matter *m);

// Audio part
daku_wave *daku_wave_create();
void daku_wave_setlife(daku_wave *w, float life_time);
void daku_wave_play(daku_wave *w, float start_time, daku_instrument *instrument);

// World part
daku_world *daku_world_create(int width, int height, float duration, int fps, int sample_rate);
void daku_world_populate(daku_world *world, daku_matter *resident, float start_time, int z_order);
void daku_world_clang(daku_world *world, daku_wave *duangduang, float start_time);
void daku_world_write(daku_world *world, const char *path);

#endif
