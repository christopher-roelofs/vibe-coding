#include "FontManager.h"
#include <iostream>

std::map<std::string, TTF_Font*> FontManager::s_fonts;

bool FontManager::init() {
    if (TTF_Init() == -1) {
        std::cerr << "Failed to initialize SDL_ttf: " << TTF_GetError() << std::endl;
        return false;
    }
    return true;
}

void FontManager::loadFont(const std::string& id, const std::string& filename, int size) {
    TTF_Font* font = TTF_OpenFont(filename.c_str(), size);
    if (!font) {
        std::cerr << "Failed to load font: " << filename << " " << TTF_GetError() << std::endl;
        return;
    }
    s_fonts[id] = font;
}

TTF_Font* FontManager::getFont(const std::string& id) {
    //std::cout << "FontManager::getFont called for ID: " << id << ". Current s_fonts.size(): " << s_fonts.size() << std::endl;
    if (s_fonts.find(id) != s_fonts.end()) {
        return s_fonts[id];
    }
    std::cerr << "Font not found: " << id << std::endl;
    return nullptr;
}

void FontManager::drawText(SDL_Renderer* renderer, const std::string& text, int x, int y, const std::string& fontId, SDL_Color color) {
    if (s_fonts.find(fontId) == s_fonts.end()) {
        std::cerr << "Font not found: " << fontId << std::endl;
        return;
    }

    SDL_Surface* surface = TTF_RenderText_Solid(s_fonts[fontId], text.c_str(), color);
    if (!surface) {
        std::cerr << "Failed to create text surface: " << TTF_GetError() << std::endl;
        return;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        std::cerr << "Failed to create text texture: " << SDL_GetError() << std::endl;
        SDL_FreeSurface(surface);
        return;
    }

    SDL_Rect destRect = { x, y, surface->w, surface->h };
    SDL_FreeSurface(surface);
    SDL_RenderCopy(renderer, texture, NULL, &destRect);
    SDL_DestroyTexture(texture);
}

void FontManager::quit() {
    //std::cout << "FontManager::quit() CALLED! Clearing " << s_fonts.size() << " fonts." << std::endl;
    for (auto const& [id, font] : s_fonts) {
        TTF_CloseFont(font);
    }
    s_fonts.clear();
    TTF_Quit();
}
