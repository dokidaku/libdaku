#ifndef __DAKU_CORE_H__
#define __DAKU_CORE_H__

#include "types.h"

daku_matter *daku_matter_create();

void daku_matter_setlife(daku_matter *m, float life_time);
void daku_matter_setsize(daku_matter *m, float width, float height);
void daku_matter_setgrosssize(daku_matter *m, float nw, float nh, float gw, float gh);
void daku_matter_setanchor(daku_matter *m, float ax, float ay);
void daku_matter_setpos(daku_matter *m, float x, float y);
void daku_matter_act(daku_matter *m, float start_time, daku_action *action);

void daku_matter_init(daku_matter *m);

daku_world *daku_world_create(float width, float height, float duration);

void daku_world_populate(daku_world *world, daku_matter *resident, float start_time);

void daku_world_write(daku_world *world, const char *path);

#endif
