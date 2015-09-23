#include "World.h"
#include <cstdlib>
#include <ctime>
extern "C" {
#include "../utils/frame-pusher.h"
}

namespace daku {

World::World(int width, int height, float duration,
    int frame_rate_num, int frame_rate_deno, int sample_rate)
: _width(width), _height(height), _duration(duration),
  _frame_rate_num(frame_rate_num), _frame_rate_deno(frame_rate_deno),
  _sample_rate(sample_rate)
{
}

uint8_t *World::getFrame(int frameIdx)
{
    static uint8_t *p = NULL;
    if (p) free(p);
    p = (uint8_t *)malloc(_width * (_height + 1) * 3);
    memset(p, 0, _width * _height * 3);
    memset(p + (rand() % _height) * _width * 3, 255, _width * 3);
    return p;
}

int World::writeToFile(const char *path)
{
    clock_t exec_start_clock = clock();
    unsigned exec_start_time = (unsigned)time(0);

    int ret;
    frame_pusher *pusher = NULL;
    if ((ret = frame_pusher_open(&pusher, path,
        _sample_rate, _frame_rate_num / _frame_rate_deno,
        _width, _height, 0)) < 0)
    {
        av_log(NULL, AV_LOG_ERROR, "daku: Cannot initialize output file\n");
        return ret;
    }

    for (int i = 0; i < _duration * (_frame_rate_num / _frame_rate_deno); ++i) {
        uint8_t *frame = this->getFrame(i);
        frame_pusher_write_video(pusher, frame, _width * 3, 1);
    }
    frame_pusher_close(pusher);

    printf("Duration: %f s\nExecution time: ~%d s\nUser time: %f s\n",
        this->_duration, ((unsigned)time(0) - exec_start_time), (double)(clock() - exec_start_clock) / CLOCKS_PER_SEC);
    return 0;
}

}
