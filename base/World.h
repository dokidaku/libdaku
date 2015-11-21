#pragma once
#ifndef __DAKU_WORLD_H__
#define __DAKU_WORLD_H__

extern "C" {
#include <libavutil/avutil.h>
#include <libswscale/swscale.h>
}
#include "./Clip.h"
#include <utility>
#include <vector>

namespace daku {

class World {
public:
    World(int width, int height, float duration,
        int frameRateNum = 30, int frameRateDeno = 1, int sampleRate = 44100);
    ~World();

    void putBoard(Clip *clip);

    uint8_t *getFrame(int frameIdx, bool seeking = false);
    int writeToFile(const char *path);

    friend class Node;

protected:
    int _width, _height;
    float _duration;
    int _frameRateNum, _frameRateDeno;
    int _sampleRate;

    //typedef std::tuple<float, Clip *, struct SwsContext *> boardData;
    // TODO: Add transitions
    struct boardData {
        float startTime;
        Clip *clip;
        // For scaling the contents of the board to fit the output.
        struct SwsContext *swsCtx;
    };
    static inline bool boardDataTimeCmp(boardData &lhs, boardData &rhs) {
        return lhs.startTime < rhs.startTime;
    }
    typedef std::vector<boardData> boardList;
    boardList _boards;
    float _boardsTotalLen;
    boardList::iterator _curClipItr;
    uint8_t *_pictBuf;
    uint16_t *_pictBuf2;
    int _pictBufLineSize;
};

}

#endif
