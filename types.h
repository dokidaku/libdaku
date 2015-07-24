#ifndef __DAKU_TYPES_H__
#define __DAKU_TYPES_H__

#include "utils/list.h"
#include <stdint.h>

struct __daku_world;
struct __daku_matter;
struct __daku_action;

typedef struct __daku_matter {
    float start_time, life_time;
    float content_width, content_height;
    float pict_width, pict_height;
    float anchor_x, anchor_y;
    float x, y, rotation;
    uint8_t *picture;
    daku_list/* struct __daku_action */ *actions;
} daku_matter;

typedef void (*daku_action_update_func)
    (struct __daku_action *action, float progress);

typedef struct __daku_action {
    struct __daku_matter *target;
    float start_time, duration;
    daku_action_update_func update;
} daku_action;

typedef struct __daku_world {
    int width, height;
    float duration;
    daku_list/* struct __daku_matter */ *population;

    int fps;
} daku_world;

#endif
