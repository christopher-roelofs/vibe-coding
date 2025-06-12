#include "Hole.h"
#include "TextureManager.h"

Hole::Hole(int grid_x, int grid_y, int tileSize)
    : GameObject(grid_x, grid_y, tileSize, tileSize, "hole", "hole") {
}

void Hole::render(SDL_Renderer* renderer, int offsetX, int offsetY) {
    TextureManager::draw("hole", m_x * m_width, m_y * m_height, m_width, m_height, renderer, offsetX, offsetY);
}
