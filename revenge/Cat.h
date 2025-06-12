#ifndef CAT_H
#define CAT_H

#include "GameObject.h"

class Level; // Forward declaration

class Cat : public GameObject {
public:
    Cat(int x, int y, int width, int height);

    void render(SDL_Renderer* renderer, int offsetX = 0, int offsetY = 0) override;
    void update(Level& level) override;

private:
    int m_moveTimer;
    static const int MOVE_DELAY = 30; // Higher value = slower cat. Moves roughly every half-second.
};

#endif // CAT_H
