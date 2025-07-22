#include "../include/Menu.h"

Menu::Menu(SDL_Renderer* renderer, TTF_Font* font) 
    : renderer(renderer), font(font), selectedIndex(0), startGame(false), quit(false),
      codeLength(6), numColors(6) {
    menuItems.push_back("Start Game");
    menuItems.push_back("Length");
    menuItems.push_back("Colors");
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
                
            case SDLK_LEFT:
                if (selectedIndex == 1) { // Length
                    if (codeLength > 3) codeLength--;
                } else if (selectedIndex == 2) { // Colors
                    if (numColors > 3) numColors--;
                }
                break;
                
            case SDLK_RIGHT:
                if (selectedIndex == 1) { // Length
                    if (codeLength < 8) codeLength++;
                } else if (selectedIndex == 2) { // Colors
                    if (numColors < 8) numColors++;
                }
                break;
                
            case SDLK_RETURN:
                if (selectedIndex == 0) {
                    startGame = true;
                } else if (selectedIndex == 3) {
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
    SDL_Color cyan = {0, 255, 255, 255};
    
    renderText("MASTERMIND", 320, 50, cyan);
    
    SDL_Surface* surface = TTF_RenderText_Solid(font, "Crack the secret code!", white);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect rect = {320 - surface->w/2, 80, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, nullptr, &rect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
    
    // Game rules
    std::string lengthRule = "- Guess the " + std::to_string(codeLength) + "-color secret code";
    const char* staticRules[] = {
        "Rules:",
        "- You have 12 attempts",
        "- Black underline = correct color and position",
        "- White underline = correct color, wrong position",
        "- Use left/right arrows to cycle colors"
    };
    
    std::vector<std::string> rules;
    rules.push_back(staticRules[0]);
    rules.push_back(lengthRule);
    for (int i = 1; i < 5; i++) {
        rules.push_back(staticRules[i]);
    }
    
    int ruleY = 110;
    for (const std::string& rule : rules) {
        SDL_Color color = (rule[0] == 'R') ? cyan : white;
        surface = TTF_RenderText_Solid(font, rule.c_str(), color);
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        rect = {320 - surface->w/2, ruleY, surface->w, surface->h};
        SDL_RenderCopy(renderer, texture, nullptr, &rect);
        SDL_FreeSurface(surface);
        SDL_DestroyTexture(texture);
        ruleY += 22;
    }
    
    // Menu items with values
    for (size_t i = 0; i < menuItems.size(); i++) {
        SDL_Color color = (i == selectedIndex) ? yellow : white;
        
        if (i == 1) { // Length option
            std::string lengthText = menuItems[i] + ": " + std::to_string(codeLength);
            renderText(lengthText, 320, 280 + i * 30, color);
            if (selectedIndex == 1) {
                renderText("<", 250, 280 + i * 30, yellow);
                renderText(">", 390, 280 + i * 30, yellow);
            }
        } else if (i == 2) { // Colors option
            std::string colorsText = menuItems[i] + ": " + std::to_string(numColors);
            renderText(colorsText, 320, 280 + i * 30, color);
            if (selectedIndex == 2) {
                renderText("<", 250, 280 + i * 30, yellow);
                renderText(">", 390, 280 + i * 30, yellow);
            }
        } else {
            renderText(menuItems[i], 320, 280 + i * 30, color);
        }
    }
    
    renderText("Use Arrow Keys to Navigate, Enter to Select", 320, 450, white);
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