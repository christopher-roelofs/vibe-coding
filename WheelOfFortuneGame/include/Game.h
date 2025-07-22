#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <memory>

class StartScreen;
class WheelOfFortune;

enum class GameState {
    START_SCREEN,
    PLAYING,
    GAME_OVER,
    QUIT
};

class Game {
public:
    Game();
    ~Game();
    
    bool Initialize();
    void Run();
    void Cleanup();
    
    void SetState(GameState newState);
    GameState GetState() const { return currentState; }
    
    SDL_Renderer* GetRenderer() const { return renderer; }
    TTF_Font* GetFont() const { return font; }
    
    static const int LOGICAL_WIDTH = 640;
    static const int LOGICAL_HEIGHT = 480;
    static const int WINDOW_WIDTH = 1024;
    static const int WINDOW_HEIGHT = 768;

private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    TTF_Font* font;
    
    GameState currentState;
    bool running;
    
    std::unique_ptr<StartScreen> startScreen;
    std::unique_ptr<WheelOfFortune> wheelOfFortune;
    
    void HandleEvents();
    void Update();
    void Render();
    
    bool LoadFont();
};