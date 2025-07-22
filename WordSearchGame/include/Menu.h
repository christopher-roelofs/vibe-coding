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
    const std::string& getSelectedTheme() const { return selectedTheme; }
    
private:
    SDL_Renderer* renderer;
    TTF_Font* font;
    
    std::vector<std::string> menuItems;
    int selectedIndex;
    bool startGame;
    bool quit;
    
    std::vector<std::string> themes;
    int themeIndex;
    std::string selectedTheme;
    
    void renderText(const std::string& text, int x, int y, SDL_Color color);
    void loadAvailableThemes();
};

#endif