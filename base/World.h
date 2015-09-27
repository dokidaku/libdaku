#pragma once
#ifndef __DAKU_WORLD_H__
#define __DAKU_WORLD_H__

extern "C" {
#include <libavutil/avutil.h>
}
#include "./Clip.h"

namespace daku {

class World {
public:
    World(int width, int height, float duration,
        int frame_rate_num = 30, int frame_rate_deno = 1, int sample_rate = 44100);
    ~World();

    void putBoard(Clip *clip);

    uint8_t *getFrame(int frameIdx);
    int writeToFile(const char *path);

protected:
    int _width, _height;
    float _duration;
    int _frame_rate_num, _frame_rate_deno;
    int _sample_rate;

    // Will be replaced later by an std::vector.
    Clip *_clip;
};

}

#endif
