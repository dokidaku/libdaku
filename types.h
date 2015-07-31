#ifndef __DAKU_TYPES_H__
#define __DAKU_TYPES_H__

#include "utils/list.h"
#include <stdint.h>

struct __daku_world;
struct __daku_matter;
struct __daku_action;
struct __daku_instrument;

// Video part
typedef struct __daku_matter {
    float start_time, life_time;
    float content_width, content_height;
    float content_start_x, content_start_y;
    float pict_width, pict_height;
    float anchor_x, anchor_y;
    float x, y, rotation;
    uint16_t opacity;
    unsigned char flipped_x, flipped_y;
    uint16_t *picture;
    daku_list/* struct __daku_action */ *actions;
} daku_matter;

typedef int (*daku_action_init_func)
    (struct __daku_action *action);
typedef void (*daku_action_update_func)
    (struct __daku_action *action, float progress);

typedef struct __daku_action {
    struct __daku_matter *target;
    float start_time, duration;
    unsigned char initialized;
    daku_action_init_func init;
    daku_action_update_func update;
} daku_action;

// Audio part
typedef struct __daku_wave {
    float start_time, life_time;
    int last_sample_idx, data_len, data_ptr;
    int sample_rate;
    int16_t *waveform_data[2];
    daku_list/* struct __daku_instrument */ *instruments;
} daku_wave;

typedef int (*daku_instrument_init_func)
    (struct __daku_instrument *wave);
typedef void (*daku_instrument_update_func)
    (struct __daku_instrument *wave, int sample_idx);

typedef struct __daku_instrument {
    struct __daku_wave *target;
    float start_time, duration;
    daku_instrument_init_func init;
    daku_instrument_update_func update;
} daku_instrument;

// World part
typedef struct __daku_world {
    int width, height;
    float duration;
    daku_list/* struct __daku_matter */ *population;
    daku_list/* struct __daku_wave */ *clangs;

    int fps;
    int sample_rate;
} daku_world;

#endif
