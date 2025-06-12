#include "Cat.h"
#include "TextureManager.h"
#include "Level.h"
#include <cstdlib> // For rand()
#include <ctime>   // For time()

Cat::Cat(int x, int y, int width, int height)
    : GameObject(x, y, width, height, "cat", "cat"), m_moveTimer(0) {
    // Seed the random number generator once
    static bool seeded = false;
    if (!seeded) {
        srand(time(0));
        seeded = true;
    }
}

void Cat::render(SDL_Renderer* renderer, int offsetX, int offsetY) {
    TextureManager::draw("cat", m_x * m_width, m_y * m_height, m_width, m_height, renderer, offsetX, offsetY);
}

void Cat::update(Level& level) {
    m_moveTimer++;

    if (m_moveTimer < MOVE_DELAY) {
        return; // Not time to move yet
    }

    m_moveTimer = 0; // Reset timer

    int moveDirection = rand() % 8; // 0-3 for cardinal, 4-7 for diagonal
    int dx = 0, dy = 0;

    switch (moveDirection) {
        // Cardinal directions
        case 0: dy = -1; break; // Up
        case 1: dy = 1;  break; // Down
        case 2: dx = -1;  break; // Left
        case 3: dx = 1;   break; // Right
        // Diagonal directions
        case 4: dx = 1;  dy = -1; break; // Up-Right
        case 5: dx = 1;  dy = 1;  break; // Down-Right
        case 6: dx = -1; dy = 1;  break; // Down-Left
        case 7: dx = -1; dy = -1; break; // Up-Left
    }

    int newX = m_x + dx;
    int newY = m_y + dy;

    // Check if the target destination is valid (not solid and not occupied).
    // This version allows moving through diagonal gaps where corners meet.
    if (!level.isTileSolid(newX, newY) && level.getGameObjectAt(newX, newY) == nullptr) {
        m_x = newX;
        m_y = newY;
    }
}
