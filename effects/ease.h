#ifndef __DAKU_EASE_H__
#define __DAKU_EASE_H__

#include "../types.h"

#define DAKU_EASE_DEF_ARG0(__name) daku_action *daku_ease_##__name(daku_action *inner)
#define DAKU_EASE_DEF_ARG1(__name) daku_action *daku_ease_##__name(daku_action *inner, float arg1)

// ease.c
DAKU_EASE_DEF_ARG0(sine_in);
DAKU_EASE_DEF_ARG0(sine_out);
DAKU_EASE_DEF_ARG0(sine_inout);

DAKU_EASE_DEF_ARG0(quad_in);
DAKU_EASE_DEF_ARG0(quad_out);
DAKU_EASE_DEF_ARG0(quad_inout);

DAKU_EASE_DEF_ARG0(cubic_in);
DAKU_EASE_DEF_ARG0(cubic_out);
DAKU_EASE_DEF_ARG0(cubic_inout);

DAKU_EASE_DEF_ARG0(quart_in);
DAKU_EASE_DEF_ARG0(quart_out);
DAKU_EASE_DEF_ARG0(quart_inout);

DAKU_EASE_DEF_ARG0(quint_in);
DAKU_EASE_DEF_ARG0(quint_out);
DAKU_EASE_DEF_ARG0(quint_inout);

DAKU_EASE_DEF_ARG0(circ_in);
DAKU_EASE_DEF_ARG0(circ_out);
DAKU_EASE_DEF_ARG0(circ_inout);

DAKU_EASE_DEF_ARG1(time_scale);

// TODO: Add parameterized exponential easing functions

#endif
