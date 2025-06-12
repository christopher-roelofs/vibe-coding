#include "Cheese.h"
#include "TextureManager.h"

Cheese::Cheese(int x, int y, int width, int height)
    : GameObject(x, y, width, height, "cheese", "cheese") {}

void Cheese::render(SDL_Renderer* renderer, int offsetX, int offsetY) {
    TextureManager::draw("cheese", m_x * m_width, m_y * m_height, m_width, m_height, renderer, offsetX, offsetY);
}

// Cheese doesn't have any complex update logic
void Cheese::update(Level& level) {}
