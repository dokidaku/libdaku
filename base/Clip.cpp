#include "./Clip.h"
#include <cstdlib>

namespace daku {

Clip::Clip(int width, int height, float lifeTime)
{
    this->init(width, height, lifeTime);
    this->_hasPicture = true;
}

Clip::~Clip()
{
    free(this->_picture);
    free(this->_pictAlpha);
}

void Clip::prepare()
{
    Node::prepare();
    int i, j;
    for (i = 0; i < 64; ++i)
        for (j = 0; j < 64; ++j)
            if (i + j < 63) this->_pictAlpha[i * this->_width + j] = (int)(65535.0 / 62.0 * (i + j));
}

void Clip::update(float time)
{
    std::vector<Node *>::iterator i;
    Node *c;
    // Update skeletons (not implemented)
    // for (i = _children.begin(); i != _children.end() && (c = *i)->_zOrder < 0; ++i)
    //     if (!c->_hasPicture && c->_startTime <= time && time <= c->_startTime + c->_lifeTime)
    //         c->update(time - c->_startTime);
    // Draw children with z-order < 0
    for (i = _children.begin(); i != _children.end() && (c = *i)->_zOrder < 0; ++i) {
        if (c->_hasPicture && c->_startTime <= time && time <= c->_startTime + c->_lifeTime)
            this->drawChild((Clip *)c, time - c->_startTime);
    }
    // Draw content
    this->draw(time);
    // Draw children with z-order >= 0
    for (; i != _children.end() && (c = *i); ++i) {
        if (c->_hasPicture && c->_startTime <= time && time <= c->_startTime + c->_lifeTime)
            this->drawChild((Clip *)c, time - c->_startTime);
    }
}

// TODO: Here's the simplest implementation. Positions, rotations etc. need to be considered.
#define ALPHA_BLEND3(__var, __val, __alpha) (__var = (double)((__var) * (65535 - (__alpha)) + (__val) * (__alpha)) / 65535.0)
void Clip::drawChild(Clip *child, float time)
{
    child->update(time);
    static int i, j;
    static uint16_t a;
    for (i = 0; i < child->_height; ++i) {
        // memcpy(this->_picture + this->_lineSize * (this->_height - 1 - i),
        //     child->_picture + child->_lineSize * i,
        //     child->_lineSize * sizeof(uint16_t));
        for (j = 0; j < child->_width; ++j) {
            a = child->_pictAlpha[i * child->_width + j];
            ALPHA_BLEND3(_picture[i * _lineSize + j * 3 + 0], child->_picture[i * child->_lineSize + j * 3 + 0], a);
            ALPHA_BLEND3(_picture[i * _lineSize + j * 3 + 1], child->_picture[i * child->_lineSize + j * 3 + 1], a);
            ALPHA_BLEND3(_picture[i * _lineSize + j * 3 + 2], child->_picture[i * child->_lineSize + j * 3 + 2], a);
            // if ((rand() & 16383) == 0)
            //     printf("%d %d %d\n", (int)this->_pictAlpha[i * _width + j], (int)a,
            //         (int)(this->_pictAlpha[i * _width + j] + (double)a / 65535.0 * (65535 - this->_pictAlpha[i * _width + j])));
            this->_pictAlpha[i * _width + j] += (double)a / 65535.0 * (65535 - this->_pictAlpha[i * _width + j]);
        }
    }
}

// TODO: For testing purposes, will be moved to a separate file later
void Clip::draw(float time)
{
    int whiteHeight = time / this->_lifeTime * this->_height;
    if (whiteHeight > this->_height) whiteHeight = this->_height;
    memset(this->_picture, 255, whiteHeight * this->_lineSize * sizeof(uint16_t));
    // Alpha = 144 * 256 + 144 = 37008
    // Alpha Mix (37008, 37008) = Alpha Mix (0.5647, 0.5647) = 0.8105 = 206.68 / 255
    memset(this->_pictAlpha, 144, whiteHeight * this->_width * sizeof(uint16_t));
    memset(this->_pictAlpha + whiteHeight * this->_width, 0, (_height - whiteHeight + 1) * this->_width * sizeof(uint16_t));
    static int x, y;
    for (int i = 0; i < 20; ++i) {
        x = rand() % this->_width;
        y = rand() % this->_height;
        if (x >= 3 && x < this->_width - 4 && y >= 3 && y < this->_height - 4) {
            static int ii, jj, tt, rr, gg, bb;
            tt = rand() & 65535;
            rr = rand() & 65535;
            gg = rand() & 65535;
            bb = rand() & 65535;
            for (ii = -3; ii <= 3; ++ii)
                for (jj = -3; jj <= 3; ++jj) {
                    this->_picture[(y + ii) * this->_lineSize + (x + jj) * 3 + 0] = rr;
                    this->_picture[(y + ii) * this->_lineSize + (x + jj) * 3 + 1] = gg;
                    this->_picture[(y + ii) * this->_lineSize + (x + jj) * 3 + 2] = bb;
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
    this->_picture = (uint16_t *)malloc((height + 1) * this->_lineSize * sizeof(uint16_t));
    this->_pictAlpha = (uint16_t *)malloc((height + 1) * this->_width * sizeof(uint16_t));
    this->_lifeTime = lifeTime;
}

}
