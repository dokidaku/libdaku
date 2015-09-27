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
    void update(float time);
    // virtual void update(float time) = 0;

protected:
};

}

#endif
