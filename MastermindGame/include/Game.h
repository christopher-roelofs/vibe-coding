#ifndef GAME_H
#define GAME_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <memory>

enum class GameState {
    MENU,
    PLAYING,
    GAME_OVER
};

class Game {
public:
    Game();
    ~Game();
    
    bool init();
    void run();
    void cleanup();
    
private:
    void handleEvents();
    void update();
    void render();
    
    SDL_Window* window;
    SDL_Renderer* renderer;
    TTF_Font* font;
    TTF_Font* smallFont;
    
    std::unique_ptr<class Menu> menu;
    std::unique_ptr<class Mastermind> mastermind;
    
    bool running;
    GameState currentState;
    
    static const int LOGICAL_WIDTH = 640;
    static const int LOGICAL_HEIGHT = 480;
};

#endif