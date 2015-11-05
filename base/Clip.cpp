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
    free(this->_pictAlpha);
}

void Clip::prepare()
{
    int i, j;
    for (i = 0; i < 64; ++i)
        for (j = 0; j < 64; ++j)
            if (i + j < 63) this->_pictAlpha[i * this->_width + j] = (int)(255.0 / 62.0 * (i + j));
}

void Clip::update(float time)
{
    int whiteHeight = time / this->_lifeTime * this->_height;
    if (whiteHeight > this->_height) whiteHeight = this->_height;
    memset(this->_picture, 255, whiteHeight * this->_lineSize);
    static int x, y;
    for (int i = 0; i < 20; ++i) {
        x = rand() % this->_width;
        y = rand() % this->_height;
        if (rand() & 1) {
            this->_picture[y * this->_lineSize + x * 3 + 0] = rand() & 255;
            this->_picture[y * this->_lineSize + x * 3 + 1] = rand() & 255;
            this->_picture[y * this->_lineSize + x * 3 + 2] = rand() & 255;
        } else {
            if (x >= 3 && x < this->_width - 4 && y >= 3 && y < this->_height - 4) {
                static int ii, jj, tt;
                tt = rand() & 255;
                for (ii = -3; ii <= 3; ++ii)
                    for (jj = -3; jj <= 3; ++jj)
                        this->_pictAlpha[(y + jj) * this->_width + (x + ii)] = tt;
            } else {
                this->_pictAlpha[y * this->_width + x] = rand() & 255;
            }
        }
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
    this->_pictAlpha = (uint8_t *)malloc((height + 1) * this->_width);
    memset(this->_pictAlpha, 255, (height + 1) * this->_width);
    this->_lifeTime = lifeTime;
}

}
