#ifndef TRAP_H
#define TRAP_H

#include "GameObject.h"
#include <string>

// Assuming TILE_SIZE is a globally accessible constant for coordinate scaling in render.
// If TILE_SIZE is defined in a specific header like "GameConstants.h", it should be included.
// extern const int TILE_SIZE; // Placeholder if it's an extern global

class Trap : public GameObject {
public:
    // Constructor takes grid coordinates (gridX, gridY)
    // and the pixel size of a tile (tilePixelSize, which will be TILE_SIZE).
    Trap(int gridX, int gridY, int tilePixelSize, const std::string& textureID = "mousetrap");
    ~Trap() override = default;

    // Renders the trap. Uses GameObject's m_x, m_y (grid coords) and m_width, m_height (pixel size).
    // Assumes a global TILE_SIZE for scaling grid coordinates to pixel coordinates for drawing.
    void render(SDL_Renderer* renderer, int offsetX = 0, int offsetY = 0) override;

    // Traps are static and likely don't need their own update logic beyond the base.
    // If they do, uncomment and implement:
    // void update(Level& level) override;
};

#endif // TRAP_H
