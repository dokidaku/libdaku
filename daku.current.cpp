// $ g++ -x c utils/frame-pusher.c -x c++ base/World.cpp daku.current.cpp `pkg-config --cflags --libs libavformat libavcodec libswscale libswresample` -D__STDC_CONSTANT_MACROS
#include "daku.h"

int main()
{
    av_register_all();
    daku::World *world = new daku::World(1280, 720, 5, 30000, 1001);    // 29.97 fps
    world->writeToFile("1.mp4");
    return 0;
}
