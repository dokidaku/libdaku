#ifndef __DAKU_MATTERS_H__
#define __DAKU_MATTERS_H__

#include "../types.h"

// shape.c
enum daku_shape_type {
    DAKU_SHAPE_RECT,
    DAKU_SHAPE_CIRCLE,
    DAKU_SHAPE_POLYGON
};
daku_action *daku_matter_shape(float duration, enum daku_shape_type type, int colour);

#endif
