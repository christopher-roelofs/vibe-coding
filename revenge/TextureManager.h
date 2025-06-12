#ifndef TEXTURE_MANAGER_H
#define TEXTURE_MANAGER_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <string>
#include <vector>
#include <map>
#include <iostream>   // For error reporting

class TextureManager {
public:
    // Texture registration and pack management
    static void registerTexture(const std::string& id, const std::string& fileName);
    static bool setGraphicsPack(const std::string& packName, SDL_Renderer* renderer);
    static std::vector<std::string> getAvailableGraphicsPacks();
    static std::string getCurrentGraphicsPackName();

    // Core texture loading and drawing
    static bool loadTexture(const std::string& id, SDL_Renderer* renderer); // fileName removed, path constructed internally
    static void draw(const std::string& id, int x, int y, int width, int height, SDL_Renderer* renderer, int offsetX = 0, int offsetY = 0, SDL_RendererFlip flip = SDL_FLIP_NONE);
    static void drawFrame(const std::string& id, int x, int y, int width, int height, int currentRow, int currentFrame, SDL_Renderer* renderer, SDL_RendererFlip flip = SDL_FLIP_NONE);
    static void clearTexture(const std::string& id);
    static void clear(); // Clears all loaded textures

private:
    // A map to hold our textures, using a string ID
    static std::map<std::string, SDL_Texture*> s_textureMap;
    // Map of texture ID to its filename (e.g., "wall" -> "wall.png")
    static std::map<std::string, std::string> s_textureFileNames;
    static std::string s_currentGraphicsPackPath; // Path to the current graphics pack (e.g., "assets/images/New/")
    static std::string s_currentGraphicsPackName; // Name of the current graphics pack (e.g., "New")
};

#endif // TEXTURE_MANAGER_H
