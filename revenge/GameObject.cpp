#include "GameObject.h"
#include <utility> // For std::move

GameObject::GameObject(int x, int y, int width, int height, const std::string& tag, std::string textureID)
    : m_x(x), m_y(y), m_width(width), m_height(height), m_textureID(std::move(textureID)), m_initialX(x), m_initialY(y), m_tag(tag) {}

#include "Level.h"

void GameObject::update(Level& level) {
    // Default update behavior: do nothing.
    // Derived classes can override this if they have specific update logic.
}

void GameObject::setPosition(int x, int y) {
    m_x = x;
    m_y = y;
}

void GameObject::resetPosition() {
    m_x = m_initialX;
    m_y = m_initialY;
}
