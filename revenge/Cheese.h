#ifndef CHEESE_H
#define CHEESE_H

#include "GameObject.h"

class Cheese : public GameObject {
public:
    Cheese(int x, int y, int width, int height);

    void render(SDL_Renderer* renderer, int offsetX = 0, int offsetY = 0) override;
    void update(Level& level) override;
};

#endif // CHEESE_H
