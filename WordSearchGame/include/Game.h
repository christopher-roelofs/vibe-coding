#ifndef GAME_H
#define GAME_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <vector>
#include <string>
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
    
    std::unique_ptr<class Menu> menu;
    std::unique_ptr<class WordSearch> wordSearch;
    
    bool running;
    GameState currentState;
    
    static const int WINDOW_WIDTH = 800;
    static const int WINDOW_HEIGHT = 600;
};

#endif