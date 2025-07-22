#ifndef MENU_H
#define MENU_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <vector>
#include <string>

class Menu {
public:
    Menu(SDL_Renderer* renderer, TTF_Font* font);
    ~Menu();
    
    void handleInput(SDL_Event& event);
    void update();
    void render();
    
    bool shouldStartGame() const { return startGame; }
    bool shouldQuit() const { return quit; }
    void reset();
    int getCodeLength() const { return codeLength; }
    int getNumColors() const { return numColors; }
    
private:
    SDL_Renderer* renderer;
    TTF_Font* font;
    
    std::vector<std::string> menuItems;
    int selectedIndex;
    bool startGame;
    bool quit;
    
    int codeLength;
    int numColors;
    
    void renderText(const std::string& text, int x, int y, SDL_Color color);
};

#endif