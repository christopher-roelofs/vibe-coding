#include "StartScreen.h"
#include "Game.h"
#include <iostream>

StartScreen::StartScreen(Game* game) : game(game) {
}

StartScreen::~StartScreen() {
}

void StartScreen::HandleEvent(const SDL_Event& event) {
    if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
            case SDLK_RETURN:
            case SDLK_SPACE:
                game->SetState(GameState::PLAYING);
                break;
            case SDLK_ESCAPE:
                game->SetState(GameState::QUIT);
                break;
        }
    }
}

void StartScreen::Update() {
}

void StartScreen::Render() {
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color yellow = {255, 255, 0, 255};
    SDL_Color lightBlue = {173, 216, 230, 255};
    
    int centerX = Game::LOGICAL_WIDTH / 2;
    
    // Title - properly centered (WHEEL OF FORTUNE ~180px, subtitle ~260px)
    RenderText("WHEEL OF FORTUNE", centerX - 90, 40, yellow);
    RenderText("A Classic Word Puzzle Game", centerX - 130, 65, lightBlue);
    
    // Two column layout with smaller font (14pt)
    int leftCol = 30;
    int rightCol = 360;
    
    // Left column - How to Play
    RenderText("HOW TO PLAY:", leftCol, 110, white);
    RenderText("- Navigate alphabet with ARROWS", leftCol + 10, 130, lightBlue);
    RenderText("- Type letters or press ENTER", leftCol + 10, 145, lightBlue);
    RenderText("- Press S to enter solve mode", leftCol + 10, 160, lightBlue);
    RenderText("- Press A to abandon puzzle", leftCol + 10, 175, lightBlue);
    
    RenderText("GOAL:", leftCol, 205, white);
    RenderText("Guess letters to reveal the phrase!", leftCol + 10, 225, lightBlue);
    RenderText("Fill in remaining letters to solve", leftCol + 10, 240, lightBlue);
    
    // Right column - Categories and Scoring
    RenderText("CATEGORIES:", rightCol, 110, white);
    RenderText("Phrases, Movies, Food", rightCol + 10, 130, lightBlue);
    RenderText("Places, People, Things", rightCol + 10, 145, lightBlue);
    
    RenderText("SCORING:", rightCol, 175, white);
    RenderText("+100 correct letters", rightCol + 10, 195, lightBlue);
    RenderText("-50 wrong letters", rightCol + 10, 210, lightBlue);
    RenderText("+1000 guess all letters", rightCol + 10, 225, lightBlue);
    RenderText("+2000 solve puzzle", rightCol + 10, 240, lightBlue);
    
    // Bottom instructions - properly centered for 640px
    RenderText("Press ENTER or SPACE to start", centerX - 105, 300, yellow);
    RenderText("Press ESCAPE to quit", centerX - 60, 330, white);
}

void StartScreen::RenderText(const char* text, int x, int y, SDL_Color color) {
    TTF_Font* font = game->GetFont();
    if (!font) return;
    
    // Use Blended rendering for anti-aliased text
    SDL_Surface* surface = TTF_RenderText_Blended(font, text, color);
    if (!surface) return;
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(game->GetRenderer(), surface);
    if (!texture) {
        SDL_FreeSurface(surface);
        return;
    }
    
    SDL_Rect destRect = {x, y, surface->w, surface->h};
    SDL_RenderCopy(game->GetRenderer(), texture, nullptr, &destRect);
    
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}