#ifndef __DAKU_EASE_H__
#define __DAKU_EASE_H__

#include "../types.h"

#define DAKU_EASE_DEF(__name) daku_action *daku_ease_##__name(daku_action *inner)

// ease.c
DAKU_EASE_DEF(sine_in);
DAKU_EASE_DEF(sine_out);
DAKU_EASE_DEF(sine_inout);

DAKU_EASE_DEF(quad_in);
DAKU_EASE_DEF(quad_out);
DAKU_EASE_DEF(quad_inout);

DAKU_EASE_DEF(cubic_in);
DAKU_EASE_DEF(cubic_out);
DAKU_EASE_DEF(cubic_inout);

DAKU_EASE_DEF(quart_in);
DAKU_EASE_DEF(quart_out);
DAKU_EASE_DEF(quart_inout);

DAKU_EASE_DEF(quint_in);
DAKU_EASE_DEF(quint_out);
DAKU_EASE_DEF(quint_inout);

DAKU_EASE_DEF(circ_in);
DAKU_EASE_DEF(circ_out);
DAKU_EASE_DEF(circ_inout);

// TODO: Add parameterized exponential easing functions

#endif
