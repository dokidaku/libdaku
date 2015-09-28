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
    memset(this->_picture, 255, whiteHeight * this->_lineSize);
    static int noisePosition;
    for (int i = 0; i < 20; ++i) {
        noisePosition = rand() % (_lineSize * _height);
        this->_picture[noisePosition + 0] = rand() & 255;
        this->_picture[noisePosition + 1] = rand() & 255;
        this->_picture[noisePosition + 2] = rand() & 255;
    }
}

void Clip::init(int width, int height, float lifeTime)
{
    this->_hasPicture = true;
    this->_width = width;
    this->_height = height;
    this->_lineSize = width * 3;
    if (this->_lineSize & 63)
        this->_lineSize = ((this->_lineSize >> 6) + 1) << 6;
    this->_picture = (uint8_t *)malloc((height + 1) * this->_lineSize);
    this->_lifeTime = lifeTime;
}

}
