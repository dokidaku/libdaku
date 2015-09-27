#include "./Node.h"

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
    this->_x = this->_y = 0;
    this->_ax = this->_ay = 0.5;
    this->_startTime = this->_lifeTime = 0;
    this->_picture = nullptr;
}

}
