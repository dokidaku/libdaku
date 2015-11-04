#include "World.h"
#include <cstdlib>
#include <ctime>
extern "C" {
#include "../utils/frame-pusher.h"
}

namespace daku {

World::World(int width, int height, float duration,
    int frameRateNum, int frameRateDeno, int sampleRate)
: _width(width), _height(height), _duration(duration),
  _frameRateNum(frameRateNum), _frameRateDeno(frameRateDeno),
  _sampleRate(sampleRate), _boardsTotalLen(0), _pictBuf(NULL), _pictBufLineSize(0)
{
    this->_pictBufLineSize = width * 3;
    if (this->_pictBufLineSize & 63)
        this->_pictBufLineSize = ((this->_pictBufLineSize >> 6) + 1) << 6;
    this->_pictBuf = (uint8_t *)malloc(height * this->_pictBufLineSize);
}

World::~World()
{
    free(this->_pictBuf);
    for (boardList::iterator i = _boards.begin(); i != _boards.end(); ++i)
        sws_freeContext(i->swsCtx);
}

void World::putBoard(Clip *clip)
{
    struct SwsContext *swsCtx = sws_getContext(
        clip->_width, clip->_height, PIX_FMT_RGB24,
        _width, _height, PIX_FMT_RGB24,
        SWS_BILINEAR, NULL, NULL, NULL);
    this->_boards.push_back((boardData){this->_boardsTotalLen, clip, swsCtx});
    this->_boardsTotalLen += clip->_lifeTime;
}

uint8_t *World::getFrame(int frameIdx)
{
    if (this->_curClipItr == this->_boards.end()) {
        memset(this->_pictBuf, 0, this->_height * this->_pictBufLineSize);
        return this->_pictBuf;
    }
    float seconds = (float)frameIdx * _frameRateDeno / _frameRateNum;
    Clip *clip = this->_curClipItr->clip;
    while (this->_curClipItr->startTime + clip->_lifeTime < seconds) {
        if (++this->_curClipItr == this->_boards.end()) {
            memset(this->_pictBuf, 0, this->_height * this->_pictBufLineSize);
            return this->_pictBuf;
        }
        (clip = this->_curClipItr->clip)->prepare();
    }
    clip->update(seconds - this->_curClipItr->startTime);
    if (clip->getWidth() == _width && clip->getHeight() == _height) {
        return clip->getPicture();
    } else {
        static const uint8_t *inPictArr[4] = { 0 };
        static uint8_t *outPictArr[4] = { 0 };
        static int inLineSizeArr[4] = { 0 };
        static int outLineSizeArr[4] = { 0 };
        inPictArr[0] = clip->getPicture();
        outPictArr[0] = this->_pictBuf;
        inLineSizeArr[0] = clip->getLineSize();
        outLineSizeArr[0] = this->_pictBufLineSize;
        sws_scale(this->_curClipItr->swsCtx,
            inPictArr, inLineSizeArr, 0, clip->getHeight(),
            outPictArr, outLineSizeArr);
        return this->_pictBuf;
    }
}

int World::writeToFile(const char *path)
{
    clock_t exec_start_clock = clock();
    unsigned exec_start_time = (unsigned)time(0);

    std::sort(this->_boards.begin(), this->_boards.end(), boardDataTimeCmp);
    this->_curClipItr = this->_boards.begin();
    this->_curClipItr->clip->prepare();

    int ret;
    frame_pusher *pusher = NULL;
    if ((ret = frame_pusher_open(&pusher, path,
        _sampleRate, (AVRational){_frameRateNum, _frameRateDeno},
        _width, _height, 0)) < 0)
    {
        av_log(NULL, AV_LOG_ERROR, "daku: Cannot initialize output file\n");
        return ret;
    }

    for (int i = 0; i < _duration * _frameRateNum / _frameRateDeno; ++i) {
        uint8_t *frame = this->getFrame(i);
        frame_pusher_write_video(pusher, frame, this->_pictBufLineSize, 1);
    }
    frame_pusher_close(pusher);

    printf("Duration: %f s\nExecution time: ~%d s\nUser time: %f s\n",
        this->_duration, ((unsigned)time(0) - exec_start_time), (double)(clock() - exec_start_clock) / CLOCKS_PER_SEC);
    return 0;
}

}
