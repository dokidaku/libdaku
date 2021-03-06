// $ g++ -x c utils/frame-pusher.c -x c++ daku.cpp base/World.cpp base/Node.cpp base/Clip.cpp daku.current.cpp `pkg-config --cflags --libs libavformat libavcodec libswscale libswresample` -D__STDC_CONSTANT_MACROS
#include "daku.h"

int main()
{
    daku::bigBang();
    daku::World *world = new daku::World(1280, 720, 5, 30000, 1001);    // 29.97 fps
    daku::Clip *clip_1 = new daku::Clip(320, 240, 1);
    daku::Clip *clip_1_2 = new daku::Clip(90, 90, 0.6);
    clip_1->addChild(clip_1_2, 0.2, 1);
    daku::Clip *clip_1_1 = new daku::Clip(180, 240, 0.6);
    clip_1->addChild(clip_1_1, 0.2, -1);    // Will not be visible
    world->putBoard(clip_1);
    daku::Clip *clip_2 = new daku::Clip(1280, 720, 2);
    world->putBoard(clip_2);
    daku::Clip *clip_3 = new daku::Clip(130, 200, 1);
    world->putBoard(clip_3);
    world->writeToFile("1.mp4");
    return 0;
}
