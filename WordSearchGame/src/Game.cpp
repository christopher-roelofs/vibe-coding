#include "../include/Game.h"
#include "../include/Menu.h"
#include "../include/WordSearch.h"
#include <iostream>

Game::Game() : window(nullptr), renderer(nullptr), font(nullptr), 
               menu(nullptr), wordSearch(nullptr),
               running(false), currentState(GameState::MENU) {
}

Game::~Game() {
    cleanup();
}

bool Game::init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }
    
    if (TTF_Init() < 0) {
        std::cerr << "SDL_ttf could not initialize! TTF_Error: " << TTF_GetError() << std::endl;
        return false;
    }
    
    window = SDL_CreateWindow("Word Search Game", 
                             SDL_WINDOWPOS_CENTERED, 
                             SDL_WINDOWPOS_CENTERED,
                             WINDOW_WIDTH, WINDOW_HEIGHT, 
                             SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }
    
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }
    
    font = TTF_OpenFont("/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf", 24);
    if (!font) {
        font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 24);
        if (!font) {
            std::cerr << "Failed to load font! TTF_Error: " << TTF_GetError() << std::endl;
            return false;
        }
    }
    
    running = true;
    return true;
}

void Game::run() {
    menu = std::make_unique<Menu>(renderer, font);
    
    while (running) {
        handleEvents();
        
        switch (currentState) {
            case GameState::MENU:
                menu->update();
                if (menu->shouldStartGame()) {
                    currentState = GameState::PLAYING;
                    wordSearch = std::make_unique<WordSearch>(renderer, font);
                    wordSearch->newGame(menu->getSelectedTheme());
                    menu->reset();
                } else if (menu->shouldQuit()) {
                    running = false;
                }
                break;
                
            case GameState::PLAYING:
                if (wordSearch) {
                    wordSearch->update();
                    if (wordSearch->wantsToQuit()) {
                        currentState = GameState::MENU;
                        wordSearch.reset();
                    } else if (wordSearch->isGameComplete()) {
                        currentState = GameState::GAME_OVER;
                    }
                }
                break;
                
            case GameState::GAME_OVER:
                break;
        }
        
        render();
    }
}

void Game::handleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            running = false;
        }
        
        switch (currentState) {
            case GameState::MENU:
                if (menu) {
                    menu->handleInput(event);
                }
                break;
                
            case GameState::PLAYING:
                if (wordSearch) {
                    wordSearch->handleInput(event);
                }
                break;
                
            case GameState::GAME_OVER:
                if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_RETURN) {
                    currentState = GameState::MENU;
                }
                break;
        }
    }
}

void Game::update() {
}

void Game::render() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    switch (currentState) {
        case GameState::MENU:
            if (menu) {
                menu->render();
            }
            break;
            
        case GameState::PLAYING:
            if (wordSearch) {
                wordSearch->render();
            }
            break;
            
        case GameState::GAME_OVER:
            {
                SDL_Color white = {255, 255, 255, 255};
                SDL_Surface* surface = TTF_RenderText_Solid(font, "Congratulations! Press Enter to continue", white);
                SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
                SDL_Rect rect = {WINDOW_WIDTH/2 - surface->w/2, WINDOW_HEIGHT/2 - surface->h/2, surface->w, surface->h};
                SDL_RenderCopy(renderer, texture, nullptr, &rect);
                SDL_FreeSurface(surface);
                SDL_DestroyTexture(texture);
            }
            break;
    }
    
    SDL_RenderPresent(renderer);
}

void Game::cleanup() {
    if (font) {
        TTF_CloseFont(font);
        font = nullptr;
    }
    
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
    
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
    
    TTF_Quit();
    SDL_Quit();
}