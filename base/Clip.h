#pragma once
#ifndef __DAKU_CLIP_H__
#define __DAKU_CLIP_H__

#include <vector>
#include "./Node.h"

namespace daku {

class Clip : public Node {
public:
    Clip(int width, int height, float lifeTime = 0);
    ~Clip();

    virtual void init(int width, int height, float lifeTime = 0);
    virtual void prepare();
    void update(float time);
    virtual void draw(float time);  // = 0;

protected:
    void drawChild(Clip *child, float time);

    friend class Node;
};

}

#endif
