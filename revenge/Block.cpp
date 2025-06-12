#include "Block.h"
#include "TextureManager.h"

Block::Block(int x, int y, int width, int height)
    : GameObject(x, y, width, height, "block", "block") {
}

void Block::render(SDL_Renderer* renderer, int offsetX, int offsetY) {
    TextureManager::draw("block", m_x * m_width, m_y * m_height, m_width, m_height, renderer, offsetX, offsetY);
}

void Block::setPosition(int x, int y) {
    m_x = x;
    m_y = y;
}
