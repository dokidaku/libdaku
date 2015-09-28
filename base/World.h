#pragma once
#ifndef __DAKU_WORLD_H__
#define __DAKU_WORLD_H__

extern "C" {
#include <libavutil/avutil.h>
#include <libswscale/swscale.h>
}
#include "./Clip.h"

namespace daku {

class World {
public:
    World(int width, int height, float duration,
        int frameRateNum = 30, int frameRateDeno = 1, int sampleRate = 44100);
    ~World();

    void putBoard(Clip *clip);

    uint8_t *getFrame(int frameIdx);
    int writeToFile(const char *path);

    friend class Node;

protected:
    int _width, _height;
    float _duration;
    int _frameRateNum, _frameRateDeno;
    int _sampleRate;

    // Will be replaced later by an std::vector.
    Clip *_clip;
    // For scaling the contents of the board to fit the output.
    struct SwsContext *_swsCtx;
    uint8_t *_pictBuf;
    int _pictBufLineSize;
};

}

#endif
