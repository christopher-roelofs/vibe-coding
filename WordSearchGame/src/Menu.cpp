#include "../include/Menu.h"
#include <fstream>
#include <algorithm>
#include <iostream>

Menu::Menu(SDL_Renderer* renderer, TTF_Font* font) 
    : renderer(renderer), font(font), selectedIndex(0), startGame(false), quit(false), themeIndex(0) {
    menuItems.push_back("Start Game");
    menuItems.push_back("Theme: ");
    menuItems.push_back("Quit");
    
    loadAvailableThemes();
    selectedTheme = themes.empty() ? "" : themes[0];
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
                
            case SDLK_LEFT:
                if (selectedIndex == 1 && !themes.empty()) {
                    if (themeIndex > 0) {
                        themeIndex--;
                    } else {
                        themeIndex = themes.size() - 1;
                    }
                    selectedTheme = themes[themeIndex];
                }
                break;
                
            case SDLK_RIGHT:
                if (selectedIndex == 1 && !themes.empty()) {
                    themeIndex = (themeIndex + 1) % themes.size();
                    selectedTheme = themes[themeIndex];
                }
                break;
                
            case SDLK_RETURN:
                if (selectedIndex == 0) {
                    startGame = true;
                } else if (selectedIndex == 2) {
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
        
        if (i == 1) {
            std::string themeText = menuItems[i] + selectedTheme;
            
            // Calculate text width to position arrows properly
            int textWidth = 0;
            int textHeight = 0;
            if (TTF_SizeText(font, themeText.c_str(), &textWidth, &textHeight) == 0) {
                renderText(themeText, 400, 300 + i * 50, color);
                
                if (selectedIndex == 1) {
                    // Position arrows with padding from the text
                    int leftArrowX = 400 - (textWidth / 2) - 30;
                    int rightArrowX = 400 + (textWidth / 2) + 15;
                    renderText("<", leftArrowX, 300 + i * 50, yellow);
                    renderText(">", rightArrowX, 300 + i * 50, yellow);
                }
            } else {
                // Fallback if size calculation fails
                renderText(themeText, 400, 300 + i * 50, color);
            }
        } else {
            renderText(menuItems[i], 400, 300 + i * 50, color);
        }
    }
    
    renderText("Use Arrow Keys to Navigate, Enter to Select", 400, 500, white);
}

void Menu::reset() {
    startGame = false;
    quit = false;
    selectedIndex = 0;
}

void Menu::loadAvailableThemes() {
    themes.clear();
    themes.push_back("Random");
    
    std::ifstream file("words.ini");
    if (!file.is_open()) {
        return;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        line.erase(std::remove_if(line.begin(), line.end(), ::isspace), line.end());
        
        if (!line.empty() && line[0] == '[' && line.back() == ']') {
            std::string themeName = line.substr(1, line.length() - 2);
            themes.push_back(themeName);
        }
    }
    
    file.close();
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