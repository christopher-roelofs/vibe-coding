#ifndef HOLE_H
#define HOLE_H

#include "GameObject.h"

class Hole : public GameObject {
public:
    Hole(int grid_x, int grid_y, int width);
    void render(SDL_Renderer* renderer, int offsetX, int offsetY) override;
};

#endif // HOLE_H
