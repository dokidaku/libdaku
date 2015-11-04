// $ g++ -x c utils/frame-pusher.c -x c++ daku.cpp base/World.cpp base/Node.cpp base/Clip.cpp daku.current.cpp `pkg-config --cflags --libs libavformat libavcodec libswscale libswresample` -D__STDC_CONSTANT_MACROS
#include "daku.h"

int main()
{
    daku::bigBang();
    daku::World *world = new daku::World(1280, 720, 5, 30000, 1001);    // 29.97 fps
    daku::Clip *clip_1 = new daku::Clip(320, 240, 1);
    world->putBoard(clip_1);
    daku::Clip *clip_2 = new daku::Clip(640, 180, 2);
    world->putBoard(clip_2);
    daku::Clip *clip_3 = new daku::Clip(30, 20, 1);
    world->putBoard(clip_3);
    world->writeToFile("1.mp4");
    return 0;
}
