#pragma once
#ifndef __DAKU_NODE_H__
#define __DAKU_NODE_H__

#include <vector>

namespace daku {

class World;

class Node {
public:
    Node();
    ~Node();

    virtual void init();
    virtual void prepare();
    virtual void update(float time) = 0;
    inline uint16_t *getPicture() { return _hasPicture ? _picture : NULL; }
    inline int getLineSize() { return _hasPicture ? _lineSize : -1; }
    inline int getWidth() { return _hasPicture ? _width : -1; }
    inline int getHeight() { return _hasPicture ? _height : -1; }

    Node *addChild(Node *child, int zOrder = 0);

    friend class World;

    static bool zOrderCmp(Node *lhs, Node *rhs) {
        return lhs->_zOrder < rhs->_zOrder;
    }

protected:
    int _width, _height;
    int _zOrder;
    double _x, _y, _ax, _ay;
    float _lifeTime;
    std::vector<Node *> _children;
    Node *_parent;

    bool _hasPicture;
    uint16_t *_picture;
    uint16_t *_pictAlpha;
    int _lineSize;
};

}

#endif
