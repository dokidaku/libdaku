#include "./Clip.h"
#include <cstdlib>

namespace daku {

Clip::Clip(int width, int height, float lifeTime)
{
    this->init(width, height, lifeTime);
}

Clip::~Clip()
{
    free(this->_picture);
}

void Clip::update(float time)
{
    int whiteHeight = time / this->_lifeTime * this->_height;
    if (whiteHeight > this->_height) whiteHeight = this->_height;
    memset(this->_picture, 255, whiteHeight * _width * 3);
    static int noisePosition;
    for (int i = 0; i < 20; ++i) {
        noisePosition = rand() % (_width * _height);
        this->_picture[noisePosition * 3 + 0] = rand() & 255;
        this->_picture[noisePosition * 3 + 1] = rand() & 255;
        this->_picture[noisePosition * 3 + 2] = rand() & 255;
    }
}

void Clip::init(int width, int height, float lifeTime)
{
    this->_hasPicture = true;
    this->_width = width;
    this->_height = height;
    this->_picture = (uint8_t *)malloc(width * (height + 1) * 3);
    this->_lifeTime = lifeTime;
}

}
