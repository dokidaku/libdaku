#include "./Node.h"
#include <algorithm>

namespace daku {

Node::Node()
{
    this->init();
}

Node::~Node()
{
}

void Node::init()
{
    this->_hasPicture = false;
    this->_width = this->_height = 0;
    this->_zOrder = 0;
    this->_x = this->_y = 0;
    this->_ax = this->_ay = 0.5;
    this->_lifeTime = 0;
    this->_parent = NULL;
    this->_picture = NULL;
}

void Node::prepare()
{
    std::sort(_children.begin(), _children.end(), Node::zOrderCmp);
}

Node *Node::addChild(Node *child, int zOrder)
{
    child->_zOrder = zOrder;
    this->_children.push_back(child);
    return child;
}

}
