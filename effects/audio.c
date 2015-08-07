#include "actions.h"
#include <stdlib.h>

int _daku_audio_reverse_init(daku_instrument *instrument)
{
    int i;
    int16_t t;
    for (i = 0; i < instrument->samples_affected / 2; ++i) {
        t = instrument->target_data[0][i];
        instrument->target_data[0][i] = instrument->target_data[0][instrument->samples_affected - i - 1];
        instrument->target_data[0][instrument->samples_affected - i - 1] = t;
        t = instrument->target_data[1][i];
        instrument->target_data[1][i] = instrument->target_data[1][instrument->samples_affected - i - 1];
        instrument->target_data[1][instrument->samples_affected - i - 1] = t;
    }
    return 0;
}
daku_instrument *daku_audio_reverse(float duration)
{
    daku_instrument *ret = (daku_instrument *)malloc(sizeof(daku_instrument));
    ret->init = &_daku_audio_reverse_init;
    ret->duration = duration;
    return ret;
}
