#pragma once
#ifndef __DAKU_NODE_H__
#define __DAKU_NODE_H__

#include <vector>

namespace daku {

class Node {
public:
    Node();
    ~Node();

    virtual void init();
    virtual void update(float time) = 0;
    inline uint8_t *getPicture() { return _hasPicture ? _picture : nullptr; }

protected:
    int _width, _height;
    double _x, _y, _ax, _ay;
    float _startTime, _lifeTime;
    std::vector<Node> _children;

    bool _hasPicture;
    uint8_t *_picture;
};

}

#endif
