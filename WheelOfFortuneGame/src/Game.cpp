#include "Game.h"
#include "StartScreen.h"
#include "WheelOfFortune.h"
#include <iostream>

Game::Game() : window(nullptr), renderer(nullptr), font(nullptr), 
               currentState(GameState::START_SCREEN), running(false) {
}

Game::~Game() {
    Cleanup();
}

bool Game::Initialize() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL initialization failed: " << SDL_GetError() << std::endl;
        return false;
    }

    if (TTF_Init() == -1) {
        std::cerr << "TTF initialization failed: " << TTF_GetError() << std::endl;
        SDL_Quit();
        return false;
    }

    window = SDL_CreateWindow("Wheel of Fortune",
                            SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED,
                            WINDOW_WIDTH, WINDOW_HEIGHT,
                            SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

    if (!window) {
        std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
        TTF_Quit();
        SDL_Quit();
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        std::cerr << "Renderer creation failed: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return false;
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    
    // Set high quality rendering hints
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2"); // Linear filtering for scaling
    
    SDL_RenderSetLogicalSize(renderer, LOGICAL_WIDTH, LOGICAL_HEIGHT);
    
    // Use integer scaling if the window size allows for clean scaling
    SDL_RenderSetIntegerScale(renderer, SDL_FALSE); // Allow non-integer scaling for smooth results

    if (!LoadFont()) {
        std::cerr << "Font loading failed" << std::endl;
        Cleanup();
        return false;
    }

    startScreen = std::make_unique<StartScreen>(this);
    wheelOfFortune = std::make_unique<WheelOfFortune>(this);

    running = true;
    return true;
}

void Game::Run() {
    while (running) {
        HandleEvents();
        Update();
        Render();
    }
}

void Game::HandleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            running = false;
            return;
        }

        switch (currentState) {
            case GameState::START_SCREEN:
                startScreen->HandleEvent(event);
                break;
            case GameState::PLAYING:
                wheelOfFortune->HandleEvent(event);
                break;
            case GameState::GAME_OVER:
                wheelOfFortune->HandleEvent(event);
                break;
            case GameState::QUIT:
                running = false;
                break;
        }
    }
}

void Game::Update() {
    switch (currentState) {
        case GameState::START_SCREEN:
            startScreen->Update();
            break;
        case GameState::PLAYING:
            wheelOfFortune->Update();
            break;
        case GameState::GAME_OVER:
            wheelOfFortune->Update();
            break;
        case GameState::QUIT:
            running = false;
            break;
    }
}

void Game::Render() {
    SDL_SetRenderDrawColor(renderer, 30, 30, 60, 255);
    SDL_RenderClear(renderer);

    switch (currentState) {
        case GameState::START_SCREEN:
            startScreen->Render();
            break;
        case GameState::PLAYING:
        case GameState::GAME_OVER:
            wheelOfFortune->Render();
            break;
        default:
            break;
    }

    SDL_RenderPresent(renderer);
}

void Game::SetState(GameState newState) {
    currentState = newState;
    
    if (newState == GameState::PLAYING) {
        wheelOfFortune->StartNewGame();
    }
}

bool Game::LoadFont() {
    const char* fontPaths[] = {
        "InterVariable.ttf",  // Try local InterVariable font first
        "data/InterVariable.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
        "/usr/share/fonts/TTF/DejaVuSans-Bold.ttf",
        "/System/Library/Fonts/Arial.ttf",
        "/Windows/Fonts/arial.ttf"
    };

    // Calculate font size based on logical resolution
    int fontSize = 16; // Slightly larger base size for better readability
    
    for (const char* fontPath : fontPaths) {
        font = TTF_OpenFont(fontPath, fontSize);
        if (font) {
            // Enable font hinting for better rendering
            TTF_SetFontHinting(font, TTF_HINTING_NORMAL);
            std::cout << "Loaded font: " << fontPath << " at size " << fontSize << std::endl;
            return true;
        }
    }

    std::cerr << "Could not load any font. TTF Error: " << TTF_GetError() << std::endl;
    return false;
}

void Game::Cleanup() {
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