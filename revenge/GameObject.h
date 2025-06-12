#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <SDL2/SDL.h>
#include <string>

class Level; // Forward-declaration

class GameObject {
public:
    GameObject(int x, int y, int width, int height, const std::string& tag, std::string textureID);
    virtual ~GameObject() {} // Virtual destructor for proper cleanup

    // Pure virtual render function - must be implemented by derived classes
    virtual void render(SDL_Renderer* renderer, int offsetX = 0, int offsetY = 0) = 0;

    // Virtual update function - can be overridden by derived classes
    virtual void update(Level& level);

    // Getters (optional, but can be useful)
    int getX() const { return m_x; }
    int getY() const { return m_y; }
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    std::string getTag() const { return m_tag; }

    // Virtual method to set position
    virtual void setPosition(int x, int y);

    void resetPosition();

protected:
    int m_x, m_y;         // Current pixel coordinates
    int m_width, m_height; // Pixel dimensions
    std::string m_textureID;
    int m_initialX, m_initialY; // Initial position // ID for TextureManager
    std::string m_tag;
};

#endif // GAMEOBJECT_H
