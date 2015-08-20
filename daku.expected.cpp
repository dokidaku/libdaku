#include <daku/daku.h>

int main()
{
    // Global initialization
    daku::bigBang();
    // World creation
    daku::World *world = new daku::World(1280, 720, 30, 5.0);
    daku::Board *board = world->board[0];

    // Main part: world populating (video composing)
    // > Load second [3, 8) of input.mp4 (1280x720)
    daku::VideoClip *vc_1 = new daku::VideoClip(5.0, "input.mp4", 3);
    vc_1->setPosition(640, 360);
    board->populate(vc_1, 0.0);
    // > Load an image
    daku::ImageClip *ic_1 = new daku::ImageClip(5, "input.jpg");
    ic_1->setOpacity(0.5);
    ic_1->setScaleX(1.5);
    ic_1->setAnchorPoint(0, 0);
    ic_1->setPosition(0, 0);
    ic_1->runAction(2.0, daku::MoveBy(3.0, -10, 0));
    board->populate(ic_1);
    // > Display some text
    daku::Label *lb_1 = new daku::Label(2, "Proudly powered by DOKIDAKU", "input.ttf", 12);
    lb_1->setAnchorPoint(1, 1);
    lb_1->setPosition(1280, 720);
    board->populate(lb_1);
    // > (Bonus) Stickman!
    // > Arguments: duration, head size (diameter), body length, body weight, arm length, arm weight, leg length, leg weight
    daku::Stickman sm_1 = new daku::Stickman(5, 20, 20, 5, 16, 5, 16, 5);
    sm_1->arm_1_joint->runAction(0.0, daku::RotateBy(1.0, 90));
    sm_1->arm_1_endpt->runAction(1.0, daku::RotateBy(1.0, 90));
    sm_1->setAnchorPoint(1, 0);
    sm_1->setPosition(1280, 0);
    board->populate(sm_1);

    // File writing
    world->writeToFile("output.mp4");
    // Cleanup & termination
    world->packUp();
    return 0;
}
