#ifndef BLOCK_H
#define BLOCK_H

#include "GameObject.h"

class Block : public GameObject {
public:
    Block(int x, int y, int width, int height);

    void render(SDL_Renderer* renderer, int offsetX = 0, int offsetY = 0) override;
    
    void setPosition(int x, int y) override;
};

#endif // BLOCK_H
