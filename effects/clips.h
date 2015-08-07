#ifndef __DAKU_CLIPS_H__
#define __DAKU_CLIPS_H__

#include "../types.h"

// clips.c
daku_action *daku_video_clip(const char *path, float start_time, float duration);
daku_action *daku_image_clip(float duration, const char *path);
daku_instrument *daku_audio_clip(const char *path, float start_time, float duration);

enum daku_text_h_align {
    DAKU_HALIGN_LEFT,
    DAKU_HALIGN_RIGHT,
    DAKU_HALIGN_CENTRE
};
daku_action *daku_text(float duration, const char *text,
    const char *path, int size, int line_height, enum daku_text_h_align h_align,
    int colour);

#endif
