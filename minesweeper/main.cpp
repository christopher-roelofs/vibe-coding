// main.cpp
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

// Screen dimensions
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

// Grid dimensions (example, can be adjusted)
const int GRID_ROWS = 10;
const int GRID_COLS = 10;
const int CELL_SIZE = 30; // Adjust for screen fit
const int NUM_MINES = 10; // Example number of mines

// Game state
enum class GameState { PLAYING, WON, LOST };
GameState currentState = GameState::PLAYING;

// Cell structure
struct Cell {
    bool isMine = false;
    bool isRevealed = false;
    bool isFlagged = false;
    int adjacentMines = 0;
};

std::vector<std::vector<Cell>> grid(GRID_ROWS, std::vector<Cell>(GRID_COLS));
int cursorX = 0;
int cursorY = 0;
TTF_Font* gameFont = nullptr;
SDL_Color textColor = {0, 0, 0, 255}; // Black

// Function declarations
void initGame();
void handleInput(SDL_Event& e, bool& quit);
void updateGame();
void renderGame(SDL_Renderer* renderer);
void closeSDL(SDL_Window* window, SDL_Renderer* renderer);
void placeMines();
void calculateAdjacentMines();
void revealCell(int r, int c);
int countFlags();
int countRevealedCells();
bool checkWinCondition();
bool checkLossCondition(int r, int c);
void renderText(SDL_Renderer* renderer, const std::string& text, int x, int y);


int main(int argc, char* args[]) {
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Initialize SDL_ttf
    if (TTF_Init() == -1) {
        std::cerr << "SDL_ttf could not initialize! TTF_Error: " << TTF_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    // Create window
    window = SDL_CreateWindow("Minesweeper", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    // Create renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == nullptr) {
        std::cerr << "Renderer could not be created! SDL Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    // Load font (ensure a font like "Arial.ttf" is in the same directory or provide a path)
    gameFont = TTF_OpenFont("VCR_OSD_MONO.ttf", 24); // Adjust font size as needed
    if (gameFont == nullptr) {
        std::cerr << "Failed to load font! TTF_Error: " << TTF_GetError() << std::endl;
        // Continue without font, or handle error more gracefully
    }

    initGame();

    bool quit = false;
    SDL_Event e;

    // Game loop
    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
            handleInput(e, quit);
        }

        if (currentState == GameState::PLAYING) {
            updateGame();
        }

        renderGame(renderer);
    }

    closeSDL(window, renderer);
    return 0;
}

void initGame() {
    srand(time(0)); // Seed random number generator
    currentState = GameState::PLAYING;
    cursorX = 0;
    cursorY = 0;

    // Reset grid
    for (int r = 0; r < GRID_ROWS; ++r) {
        for (int c = 0; c < GRID_COLS; ++c) {
            grid[r][c] = Cell();
        }
    }
    placeMines();
    calculateAdjacentMines();
}

void placeMines() {
    int minesPlaced = 0;
    while (minesPlaced < NUM_MINES) {
        int r = rand() % GRID_ROWS;
        int c = rand() % GRID_COLS;
        if (!grid[r][c].isMine) {
            grid[r][c].isMine = true;
            minesPlaced++;
        }
    }
}

void calculateAdjacentMines() {
    for (int r = 0; r < GRID_ROWS; ++r) {
        for (int c = 0; c < GRID_COLS; ++c) {
            if (grid[r][c].isMine) continue;
            int count = 0;
            for (int dr = -1; dr <= 1; ++dr) {
                for (int dc = -1; dc <= 1; ++dc) {
                    if (dr == 0 && dc == 0) continue;
                    int nr = r + dr;
                    int nc = c + dc;
                    if (nr >= 0 && nr < GRID_ROWS && nc >= 0 && nc < GRID_COLS && grid[nr][nc].isMine) {
                        count++;
                    }
                }
            }
            grid[r][c].adjacentMines = count;
        }
    }
}

void revealCell(int r, int c) {
    if (r < 0 || r >= GRID_ROWS || c < 0 || c >= GRID_COLS || grid[r][c].isRevealed || grid[r][c].isFlagged) {
        return;
    }
    grid[r][c].isRevealed = true;

    if (grid[r][c].isMine) {
        currentState = GameState::LOST;
        // Optionally reveal all mines
        for(int i=0; i<GRID_ROWS; ++i) for(int j=0; j<GRID_COLS; ++j) if(grid[i][j].isMine) grid[i][j].isRevealed = true;
        return;
    }

    if (grid[r][c].adjacentMines == 0) {
        // Reveal adjacent cells recursively if this cell has 0 adjacent mines
        for (int dr = -1; dr <= 1; ++dr) {
            for (int dc = -1; dc <= 1; ++dc) {
                if (dr == 0 && dc == 0) continue;
                revealCell(r + dr, c + dc);
            }
        }
    }
     if (checkWinCondition()) {
        currentState = GameState::WON;
    }
}

int countFlags() {
    int flagCount = 0;
    for (int r = 0; r < GRID_ROWS; ++r) {
        for (int c = 0; c < GRID_COLS; ++c) {
            if (grid[r][c].isFlagged) {
                flagCount++;
            }
        }
    }
    return flagCount;
}

int countRevealedCells() {
    int revealedCount = 0;
    for (int r = 0; r < GRID_ROWS; ++r) {
        for (int c = 0; c < GRID_COLS; ++c) {
            if (grid[r][c].isRevealed) {
                revealedCount++;
            }
        }
    }
    return revealedCount;
}


bool checkWinCondition() {
    int revealedCount = countRevealedCells();
    return revealedCount == (GRID_ROWS * GRID_COLS - NUM_MINES);
}

bool checkLossCondition(int r, int c) {
    return grid[r][c].isMine && grid[r][c].isRevealed;
}


void handleInput(SDL_Event& e, bool& quit) {
    if (e.type == SDL_KEYDOWN) {
        if (currentState == GameState::PLAYING) {
            switch (e.key.keysym.sym) {
                case SDLK_UP:
                    cursorY = (cursorY - 1 + GRID_ROWS) % GRID_ROWS;
                    break;
                case SDLK_DOWN:
                    cursorY = (cursorY + 1) % GRID_ROWS;
                    break;
                case SDLK_LEFT:
                    cursorX = (cursorX - 1 + GRID_COLS) % GRID_COLS;
                    break;
                case SDLK_RIGHT:
                    cursorX = (cursorX + 1) % GRID_COLS;
                    break;
                case SDLK_SPACE: // Reveal cell
                case SDLK_RETURN:
                    if (!grid[cursorY][cursorX].isFlagged) {
                        revealCell(cursorY, cursorX);
                    }
                    break;
                case SDLK_f: // Flag/unflag cell
                    if (!grid[cursorY][cursorX].isRevealed) {
                        grid[cursorY][cursorX].isFlagged = !grid[cursorY][cursorX].isFlagged;
                    }
                    break;
            }
        }
        // Common controls regardless of game state
        switch (e.key.keysym.sym) {
            case SDLK_r: // Restart game
                initGame();
                break;
            case SDLK_ESCAPE: // Quit
                quit = true;
                break;
        }
    }
}

void updateGame() {
    // Game logic updates go here
    // For Minesweeper, most logic is event-driven (handled in revealCell)
    // Check for win/loss conditions that might not be immediately triggered by revealCell
    if (currentState == GameState::PLAYING) {
        if (checkWinCondition()) {
            currentState = GameState::WON;
        }
        // Loss is typically handled directly in revealCell when a mine is hit.
    }
}

void renderText(SDL_Renderer* renderer, const std::string& text, int x, int y) {
    if (gameFont == nullptr || text.empty()) {
        return;
    }
    SDL_Surface* textSurface = TTF_RenderText_Solid(gameFont, text.c_str(), textColor);
    if (textSurface == nullptr) {
        std::cerr << "Unable to render text surface! TTF_Error: " << TTF_GetError() << std::endl;
    } else {
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        if (textTexture == nullptr) {
            std::cerr << "Unable to create texture from rendered text! SDL_Error: " << SDL_GetError() << std::endl;
        } else {
            SDL_Rect renderQuad = {x, y, textSurface->w, textSurface->h};
            SDL_RenderCopy(renderer, textTexture, nullptr, &renderQuad);
            SDL_DestroyTexture(textTexture);
        }
        SDL_FreeSurface(textSurface);
    }
}


void renderGame(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255); // Light grey background
    SDL_RenderClear(renderer);

    // Calculate offset to center the grid
    int gridWidth = GRID_COLS * CELL_SIZE;
    int gridHeight = GRID_ROWS * CELL_SIZE;
    int offsetX = (SCREEN_WIDTH - gridWidth) / 2;
    int offsetY = (SCREEN_HEIGHT - gridHeight) / 2;


    // Draw grid and cells
    for (int r = 0; r < GRID_ROWS; ++r) {
        for (int c = 0; c < GRID_COLS; ++c) {
            SDL_Rect cellRect = {offsetX + c * CELL_SIZE, offsetY + r * CELL_SIZE, CELL_SIZE, CELL_SIZE};

            if (grid[r][c].isRevealed) {
                if (grid[r][c].isMine) {
                    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Red for mine
                    SDL_RenderFillRect(renderer, &cellRect);
                    renderText(renderer, "M", cellRect.x + CELL_SIZE / 4, cellRect.y + CELL_SIZE / 4);
                } else {
                    SDL_SetRenderDrawColor(renderer, 180, 180, 180, 255); // Lighter grey for revealed empty
                    SDL_RenderFillRect(renderer, &cellRect);
                    if (grid[r][c].adjacentMines > 0) {
                        renderText(renderer, std::to_string(grid[r][c].adjacentMines), cellRect.x + CELL_SIZE / 3, cellRect.y + CELL_SIZE / 4);
                    }
                }
            } else if (grid[r][c].isFlagged) {
                SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); // Yellow for flagged
                SDL_RenderFillRect(renderer, &cellRect);
                renderText(renderer, "F", cellRect.x + CELL_SIZE / 4, cellRect.y + CELL_SIZE / 4);
            } else {
                SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255); // Dark grey for hidden
                SDL_RenderFillRect(renderer, &cellRect);
            }

            // Draw cell border
            SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255); // Darker border
            SDL_RenderDrawRect(renderer, &cellRect);
        }
    }

    // Draw cursor
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // Green cursor
    SDL_Rect cursorRect = {offsetX + cursorX * CELL_SIZE, offsetY + cursorY * CELL_SIZE, CELL_SIZE, CELL_SIZE};
    SDL_RenderDrawRect(renderer, &cursorRect); // Draw only border for cursor
    // Make cursor thicker
    SDL_Rect cursorRectInner = {offsetX + cursorX * CELL_SIZE + 1, offsetY + cursorY * CELL_SIZE + 1, CELL_SIZE - 2, CELL_SIZE - 2};
    SDL_RenderDrawRect(renderer, &cursorRectInner);


    // Display game state message
    std::string statusText = "";
    int flagsRemaining = NUM_MINES - countFlags();
    statusText = "Flags: " + std::to_string(flagsRemaining);
    renderText(renderer, statusText, 10, 10);


    if (currentState == GameState::WON) {
        renderText(renderer, "YOU WON! Press R to Restart", SCREEN_WIDTH / 2 - 150, SCREEN_HEIGHT - 50);
    } else if (currentState == GameState::LOST) {
        renderText(renderer, "GAME OVER! Press R to Restart", SCREEN_WIDTH / 2 - 150, SCREEN_HEIGHT - 50);
    }


    SDL_RenderPresent(renderer);
}

void closeSDL(SDL_Window* window, SDL_Renderer* renderer) {
    if (gameFont != nullptr) {
        TTF_CloseFont(gameFont);
        gameFont = nullptr;
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    renderer = nullptr;
    window = nullptr;

    TTF_Quit();
    SDL_Quit();
}
