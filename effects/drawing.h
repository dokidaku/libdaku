#ifndef __DAKU_DRAWING_H__
#define __DAKU_DRAWING_H__

#include "../types.h"

// drawing.c
enum daku_shape_type {
    DAKU_SHAPE_RECT,
    DAKU_SHAPE_ELLIPSE,
    DAKU_SHAPE_POLYGON
};
daku_action *daku_shape(float duration, enum daku_shape_type type, int colour, int opacity);

#endif
