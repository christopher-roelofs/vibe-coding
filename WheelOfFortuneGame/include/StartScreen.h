#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

class Game;

class StartScreen {
public:
    StartScreen(Game* game);
    ~StartScreen();
    
    void HandleEvent(const SDL_Event& event);
    void Update();
    void Render();

private:
    Game* game;
    
    void RenderText(const char* text, int x, int y, SDL_Color color);
};