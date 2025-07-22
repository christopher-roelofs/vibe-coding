#include "../include/Game.h"
#include "../include/Menu.h"
#include "../include/Mastermind.h"
#include <iostream>

Game::Game() : window(nullptr), renderer(nullptr), font(nullptr), smallFont(nullptr),
               menu(nullptr), mastermind(nullptr),
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
    
    // Create window that can be resized
    window = SDL_CreateWindow("Mastermind", 
                             SDL_WINDOWPOS_CENTERED, 
                             SDL_WINDOWPOS_CENTERED,
                             LOGICAL_WIDTH, LOGICAL_HEIGHT, 
                             SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (!window) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }
    
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }
    
    // Set logical rendering size for scaling
    if (SDL_RenderSetLogicalSize(renderer, LOGICAL_WIDTH, LOGICAL_HEIGHT) != 0) {
        std::cerr << "Failed to set logical rendering size! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }
    
    // Set scaling quality to linear for smooth scaling
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
    
    font = TTF_OpenFont("/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf", 24);
    if (!font) {
        font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 24);
        if (!font) {
            std::cerr << "Failed to load font! TTF_Error: " << TTF_GetError() << std::endl;
            return false;
        }
    }
    
    smallFont = TTF_OpenFont("/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf", 16);
    if (!smallFont) {
        smallFont = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 16);
        if (!smallFont) {
            std::cerr << "Failed to load small font! TTF_Error: " << TTF_GetError() << std::endl;
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
        update();
        render();
    }
}

void Game::handleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            running = false;
        }
        
        // Global hotkeys
        if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_F11) {
                // Toggle fullscreen
                Uint32 flags = SDL_GetWindowFlags(window);
                if (flags & SDL_WINDOW_FULLSCREEN_DESKTOP) {
                    SDL_SetWindowFullscreen(window, 0);
                } else {
                    SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
                }
            }
        }
        
        switch (currentState) {
            case GameState::MENU:
                if (menu) {
                    menu->handleInput(event);
                }
                break;
                
            case GameState::PLAYING:
                if (mastermind) {
                    mastermind->handleInput(event);
                }
                break;
                
            case GameState::GAME_OVER:
                if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_RETURN) {
                    currentState = GameState::MENU;
                    mastermind.reset();
                }
                break;
        }
    }
}

void Game::update() {
    switch (currentState) {
        case GameState::MENU:
            if (menu) {
                menu->update();
                if (menu->shouldStartGame()) {
                    currentState = GameState::PLAYING;
                    mastermind = std::make_unique<Mastermind>(renderer, font, smallFont, 
                                                            menu->getCodeLength(), menu->getNumColors());
                    mastermind->newGame();
                    menu->reset();
                } else if (menu->shouldQuit()) {
                    running = false;
                }
            }
            break;
            
        case GameState::PLAYING:
            if (mastermind) {
                mastermind->update();
                if (mastermind->wantsToQuit()) {
                    currentState = GameState::MENU;
                    mastermind.reset();
                } else if (mastermind->isGameWon() || mastermind->isGameLost()) {
                    currentState = GameState::GAME_OVER;
                }
            }
            break;
            
        case GameState::GAME_OVER:
            break;
    }
}

void Game::render() {
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    SDL_RenderClear(renderer);
    
    switch (currentState) {
        case GameState::MENU:
            if (menu) {
                menu->render();
            }
            break;
            
        case GameState::PLAYING:
            if (mastermind) {
                mastermind->render();
            }
            break;
            
        case GameState::GAME_OVER:
            if (mastermind) {
                mastermind->render();
                
                SDL_Color white = {255, 255, 255, 255};
                const char* text = mastermind->isGameWon() ? 
                    "Congratulations! You cracked the code! Press Enter to continue" :
                    "Game Over! Press Enter to continue";
                    
                SDL_Surface* surface = TTF_RenderText_Solid(font, text, white);
                SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
                SDL_Rect rect = {LOGICAL_WIDTH/2 - surface->w/2, LOGICAL_HEIGHT - 60, surface->w, surface->h};
                SDL_RenderCopy(renderer, texture, nullptr, &rect);
                SDL_FreeSurface(surface);
                SDL_DestroyTexture(texture);
            }
            break;
    }
    
    SDL_RenderPresent(renderer);
}

void Game::cleanup() {
    if (smallFont) {
        TTF_CloseFont(smallFont);
        smallFont = nullptr;
    }
    
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