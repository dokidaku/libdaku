#ifndef __DAKU_CLIPS_H__
#define __DAKU_CLIPS_H__

#include "../types.h"

// videoclip.c
daku_action *daku_video_clip(const char *path, float start_time, float duration);

daku_action *daku_text(float duration, const char *text, const char *path, int size, int line_height);

#endif