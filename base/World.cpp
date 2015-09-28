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
  _sampleRate(sampleRate),
  _clip(nullptr), _swsCtx(nullptr), _pictBuf(nullptr), _pictBufLineSize(0)
{
    this->_pictBufLineSize = width * 3;
    this->_pictBuf = (uint8_t *)malloc(height * this->_pictBufLineSize);
}

World::~World()
{
    free(this->_pictBuf);
    sws_freeContext(this->_swsCtx);
}

void World::putBoard(Clip *clip)
{
    this->_clip = clip;
    this->_swsCtx = sws_getContext(
        clip->_width, clip->_height, PIX_FMT_RGB24,
        _width, _height, PIX_FMT_RGB24,
        SWS_BILINEAR, NULL, NULL, NULL);
}

uint8_t *World::getFrame(int frameIdx)
{
    this->_clip->update((float)frameIdx * _frameRateDeno / _frameRateNum);
    if (this->_clip->getWidth() == _width && this->_clip->getHeight() == _height) {
        return this->_clip->getPicture();
    } else {
        static const uint8_t *inPictArr[8] = { 0 };
        static uint8_t *outPictArr[8] = { 0 };
        static int inLineSizeArr[8] = { 0 };
        static int outLineSizeArr[8] = { 0 };
        inPictArr[0] = this->_clip->getPicture();
        outPictArr[0] = this->_pictBuf;
        inLineSizeArr[0] = this->_clip->getLineSize();
        outLineSizeArr[0] = this->_pictBufLineSize;
        sws_scale(this->_swsCtx,
            inPictArr, inLineSizeArr, 0, this->_clip->getHeight(),
            outPictArr, outLineSizeArr);
        return this->_pictBuf;
    }
}

int World::writeToFile(const char *path)
{
    clock_t exec_start_clock = clock();
    unsigned exec_start_time = (unsigned)time(0);

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
        frame_pusher_write_video(pusher, frame, _width * 3, 1);
    }
    frame_pusher_close(pusher);

    printf("Duration: %f s\nExecution time: ~%d s\nUser time: %f s\n",
        this->_duration, ((unsigned)time(0) - exec_start_time), (double)(clock() - exec_start_clock) / CLOCKS_PER_SEC);
    return 0;
}

}
