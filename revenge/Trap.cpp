#include "Trap.h"
#include "TextureManager.h"

// The constructor now takes grid coordinates and passes them directly to the base class.
// The base GameObject class stores grid coordinates (m_x, m_y) and tile size (m_width, m_height).
Trap::Trap(int gridX, int gridY, int tilePixelSize, const std::string& textureID)
    : GameObject(gridX, gridY, tilePixelSize, tilePixelSize, "trap", textureID) {
    // The base class constructor handles all initialization.
}

// Renders the trap by converting its grid coordinates to pixel coordinates.
void Trap::render(SDL_Renderer* renderer, int offsetX, int offsetY) {
    // Convert grid coordinates (m_x, m_y) to pixel coordinates for rendering.
    // m_width and m_height are used as the tile size here.
    int renderX = m_x * m_width;
    int renderY = m_y * m_height;
    TextureManager::draw(m_textureID, renderX, renderY, m_width, m_height, renderer, offsetX, offsetY);
}
