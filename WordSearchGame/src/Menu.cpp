#include "../include/Menu.h"

Menu::Menu(SDL_Renderer* renderer, TTF_Font* font) 
    : renderer(renderer), font(font), selectedIndex(0), startGame(false), quit(false) {
    menuItems.push_back("Start Game");
    menuItems.push_back("Quit");
}

Menu::~Menu() {
}

void Menu::handleInput(SDL_Event& event) {
    if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
            case SDLK_UP:
                if (selectedIndex > 0) {
                    selectedIndex--;
                }
                break;
                
            case SDLK_DOWN:
                if (selectedIndex < static_cast<int>(menuItems.size()) - 1) {
                    selectedIndex++;
                }
                break;
                
            case SDLK_RETURN:
                if (selectedIndex == 0) {
                    startGame = true;
                } else if (selectedIndex == 1) {
                    quit = true;
                }
                break;
        }
    }
}

void Menu::update() {
}

void Menu::render() {
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color yellow = {255, 255, 0, 255};
    
    renderText("WORD SEARCH", 400, 150, white);
    
    for (size_t i = 0; i < menuItems.size(); i++) {
        SDL_Color color = (i == selectedIndex) ? yellow : white;
        renderText(menuItems[i], 400, 300 + i * 50, color);
    }
    
    renderText("Use Arrow Keys to Navigate, Enter to Select", 400, 500, white);
}

void Menu::reset() {
    startGame = false;
    quit = false;
    selectedIndex = 0;
}

void Menu::renderText(const std::string& text, int x, int y, SDL_Color color) {
    SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), color);
    if (!surface) return;
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        SDL_FreeSurface(surface);
        return;
    }
    
    SDL_Rect rect = {x - surface->w/2, y - surface->h/2, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, nullptr, &rect);
    
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}