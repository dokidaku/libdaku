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
  _sampleRate(sampleRate)
{
}

void World::putBoard(Clip *clip)
{
    this->_clip = clip;
}

uint8_t *World::getFrame(int frameIdx)
{
    this->_clip->update((float)frameIdx * _frameRateDeno / _frameRateNum);
    return this->_clip->getPicture();
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
