#include "../include/WordSearch.h"
#include <algorithm>
#include <random>
#include <ctime>
#include <fstream>
#include <iostream>

WordSearch::WordSearch(SDL_Renderer* renderer, TTF_Font* font, int gridSize)
    : renderer(renderer), font(font), gridSize(gridSize), 
      cursorX(0), cursorY(0), selecting(false), selectStartX(0), selectStartY(0), quit(false) {
    grid.resize(gridSize, std::vector<Cell>(gridSize, {'A', false, false}));
    srand(time(nullptr));
    
    if (!loadWordList("words.txt")) {
        std::cerr << "Warning: Could not load words.txt, using default word list" << std::endl;
        wordPool = {"HELLO", "WORLD", "GAME", "SEARCH", "WORD", "FIND", "PUZZLE", "CODE",
                    "COMPUTER", "PROGRAM", "ALGORITHM", "FUNCTION", "VARIABLE", "ARRAY"};
    }
}

WordSearch::~WordSearch() {
}

void WordSearch::handleInput(SDL_Event& event) {
    if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
            case SDLK_UP:
                if (cursorY > 0) cursorY--;
                break;
                
            case SDLK_DOWN:
                if (cursorY < gridSize - 1) cursorY++;
                break;
                
            case SDLK_LEFT:
                if (cursorX > 0) cursorX--;
                break;
                
            case SDLK_RIGHT:
                if (cursorX < gridSize - 1) cursorX++;
                break;
                
            case SDLK_RETURN:
                if (!selecting) {
                    selecting = true;
                    selectStartX = cursorX;
                    selectStartY = cursorY;
                    grid[cursorY][cursorX].selected = true;
                } else {
                    // Ensure the entire path from start to current cursor is selected
                    int dx = abs(cursorX - selectStartX);
                    int dy = abs(cursorY - selectStartY);
                    
                    if (dx == 0 || dy == 0 || dx == dy) {
                        // Clear previous selection
                        for (auto& row : grid) {
                            for (auto& cell : row) {
                                if (!cell.found) {
                                    cell.selected = false;
                                }
                            }
                        }
                        
                        // Mark the entire path as selected
                        int stepX = (cursorX > selectStartX) ? 1 : (cursorX < selectStartX) ? -1 : 0;
                        int stepY = (cursorY > selectStartY) ? 1 : (cursorY < selectStartY) ? -1 : 0;
                        
                        int x = selectStartX;
                        int y = selectStartY;
                        
                        while (true) {
                            if (x >= 0 && x < gridSize && y >= 0 && y < gridSize) {
                                grid[y][x].selected = true;
                            }
                            
                            if (x == cursorX && y == cursorY) break;
                            
                            x += stepX;
                            y += stepY;
                            
                            // Safety check
                            if (x < 0 || x >= gridSize || y < 0 || y >= gridSize) break;
                        }
                    }
                    
                    checkSelection();  // Check selection after ensuring full path is selected
                    selecting = false;
                    
                    // Clear selection after checking
                    for (auto& row : grid) {
                        for (auto& cell : row) {
                            if (!cell.found) {
                                cell.selected = false;
                            }
                        }
                    }
                }
                break;
                
            case SDLK_BACKSPACE:
                quit = true;
                break;
        }
        
        if (selecting && event.key.keysym.sym >= SDLK_UP && event.key.keysym.sym <= SDLK_RIGHT) {
            int dx = abs(cursorX - selectStartX);
            int dy = abs(cursorY - selectStartY);
            
            if (dx == 0 || dy == 0 || dx == dy) {
                for (auto& row : grid) {
                    for (auto& cell : row) {
                        if (!cell.found) {
                            cell.selected = false;
                        }
                    }
                }
                
                int stepX = (cursorX > selectStartX) ? 1 : (cursorX < selectStartX) ? -1 : 0;
                int stepY = (cursorY > selectStartY) ? 1 : (cursorY < selectStartY) ? -1 : 0;
                
                int x = selectStartX;
                int y = selectStartY;
                int steps = 0;
                const int maxSteps = gridSize * 2;
                
                while (steps < maxSteps && x >= 0 && x < gridSize && y >= 0 && y < gridSize) {
                    grid[y][x].selected = true;
                    
                    // Check if we've reached the cursor
                    if (x == cursorX && y == cursorY) {
                        break;
                    }
                    
                    // Move to next position
                    x += stepX;
                    y += stepY;
                    steps++;
                }
            }
        }
    }
}

void WordSearch::update() {
}

void WordSearch::render() {
    renderGrid();
    renderWordList();
}

void WordSearch::newGame() {
    wordsToFind.clear();
    foundWords.clear();
    
    selectRandomWords(8);
    
    generateGrid();
}

bool WordSearch::isGameComplete() const {
    return foundWords.size() == wordsToFind.size();
}

void WordSearch::generateGrid() {
    for (auto& row : grid) {
        for (auto& cell : row) {
            cell = {' ', false, false};
        }
    }
    
    for (const auto& word : wordsToFind) {
        placeWord(word);
    }
    
    fillEmptyCells();
}

void WordSearch::placeWord(const std::string& word) {
    std::vector<std::pair<int, int>> directions = {
        {1, 0}, {-1, 0}, {0, 1}, {0, -1},
        {1, 1}, {1, -1}, {-1, 1}, {-1, -1}
    };
    
    bool placed = false;
    int attempts = 0;
    
    while (!placed && attempts < 100) {
        int x = rand() % gridSize;
        int y = rand() % gridSize;
        auto dir = directions[rand() % directions.size()];
        
        bool canPlace = true;
        int endX = x + dir.first * (word.length() - 1);
        int endY = y + dir.second * (word.length() - 1);
        
        if (endX < 0 || endX >= gridSize || endY < 0 || endY >= gridSize) {
            canPlace = false;
        } else {
            for (size_t i = 0; i < word.length(); i++) {
                int checkX = x + dir.first * i;
                int checkY = y + dir.second * i;
                // Verify bounds for each position
                if (checkX < 0 || checkX >= gridSize || checkY < 0 || checkY >= gridSize) {
                    canPlace = false;
                    break;
                }
                if (grid[checkY][checkX].letter != ' ' && grid[checkY][checkX].letter != word[i]) {
                    canPlace = false;
                    break;
                }
            }
        }
        
        if (canPlace) {
            for (size_t i = 0; i < word.length(); i++) {
                int placeX = x + dir.first * i;
                int placeY = y + dir.second * i;
                // Double-check bounds before placing
                if (placeX >= 0 && placeX < gridSize && placeY >= 0 && placeY < gridSize) {
                    grid[placeY][placeX].letter = word[i];
                }
            }
            placed = true;
        }
        
        attempts++;
    }
}

void WordSearch::fillEmptyCells() {
    for (auto& row : grid) {
        for (auto& cell : row) {
            if (cell.letter == ' ') {
                cell.letter = 'A' + (rand() % 26);
            }
        }
    }
}

void WordSearch::checkSelection() {
    // Store which cells are currently selected before we check
    std::vector<std::pair<int, int>> selectedCells;
    for (int y = 0; y < gridSize; y++) {
        for (int x = 0; x < gridSize; x++) {
            if (grid[y][x].selected) {
                selectedCells.push_back({x, y});
            }
        }
    }
    
    std::string word = getSelectedWord();
    std::string reversedWord = word;
    std::reverse(reversedWord.begin(), reversedWord.end());
    
    bool foundForward = std::find(wordsToFind.begin(), wordsToFind.end(), word) != wordsToFind.end();
    bool foundReverse = std::find(wordsToFind.begin(), wordsToFind.end(), reversedWord) != wordsToFind.end();
    
    if ((foundForward || foundReverse) && 
        std::find(foundWords.begin(), foundWords.end(), word) == foundWords.end() &&
        std::find(foundWords.begin(), foundWords.end(), reversedWord) == foundWords.end()) {
        
        // Add the word that was actually in the list
        if (foundForward) {
            foundWords.push_back(word);
        } else {
            foundWords.push_back(reversedWord);
        }
        
        // Mark all cells that were selected as found
        for (const auto& pos : selectedCells) {
            grid[pos.second][pos.first].found = true;
        }
    }
}

std::string WordSearch::getSelectedWord() {
    std::string word;
    
    int dx = (cursorX > selectStartX) ? 1 : (cursorX < selectStartX) ? -1 : 0;
    int dy = (cursorY > selectStartY) ? 1 : (cursorY < selectStartY) ? -1 : 0;
    
    // Handle case where start and end are the same
    if (dx == 0 && dy == 0) {
        word += grid[selectStartY][selectStartX].letter;
        return word;
    }
    
    int x = selectStartX;
    int y = selectStartY;
    int steps = 0;
    const int maxSteps = gridSize * 2; // Safety limit
    
    while (steps < maxSteps && x >= 0 && x < gridSize && y >= 0 && y < gridSize) {
        word += grid[y][x].letter;
        if (x == cursorX && y == cursorY) break;
        
        // Calculate next position and check if it would be out of bounds
        int nextX = x + dx;
        int nextY = y + dy;
        if (nextX < 0 || nextX >= gridSize || nextY < 0 || nextY >= gridSize) {
            // Next position would be out of bounds, stop here unless we've reached cursor
            if (nextX != cursorX || nextY != cursorY) {
                break;
            }
        }
        
        x = nextX;
        y = nextY;
        steps++;
    }
    
    return word;
}

void WordSearch::renderGrid() {
    int gridX = 50;
    int gridY = 50;
    
    for (int y = 0; y < gridSize; y++) {
        for (int x = 0; x < gridSize; x++) {
            renderCell(gridX + x * CELL_SIZE, gridY + y * CELL_SIZE, grid[y][x]);
            
            if (x == cursorX && y == cursorY) {
                SDL_Rect rect = {gridX + x * CELL_SIZE, gridY + y * CELL_SIZE, CELL_SIZE, CELL_SIZE};
                SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
                SDL_RenderDrawRect(renderer, &rect);
                SDL_RenderDrawRect(renderer, &rect);
            }
        }
    }
}

void WordSearch::renderWordList() {
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color green = {0, 255, 0, 255};
    
    int listX = 550;
    int listY = 50;
    
    SDL_Surface* surface = TTF_RenderText_Solid(font, "Words to Find:", white);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect rect = {listX, listY, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, nullptr, &rect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
    
    for (size_t i = 0; i < wordsToFind.size(); i++) {
        bool found = std::find(foundWords.begin(), foundWords.end(), wordsToFind[i]) != foundWords.end();
        SDL_Color color = found ? green : white;
        
        surface = TTF_RenderText_Solid(font, wordsToFind[i].c_str(), color);
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        rect = {listX, listY + 40 + static_cast<int>(i) * 30, surface->w, surface->h};
        SDL_RenderCopy(renderer, texture, nullptr, &rect);
        SDL_FreeSurface(surface);
        SDL_DestroyTexture(texture);
    }
    
    surface = TTF_RenderText_Solid(font, "Backspace: Menu", white);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    rect = {listX, 500, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, nullptr, &rect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

void WordSearch::renderCell(int x, int y, const Cell& cell) {
    SDL_Rect rect = {x, y, CELL_SIZE, CELL_SIZE};
    
    if (cell.found) {
        SDL_SetRenderDrawColor(renderer, 0, 150, 0, 255);  // Brighter green
        SDL_RenderFillRect(renderer, &rect);
    } else if (cell.selected) {
        SDL_SetRenderDrawColor(renderer, 50, 50, 100, 255);
        SDL_RenderFillRect(renderer, &rect);
    }
    
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    SDL_RenderDrawRect(renderer, &rect);
    
    char text[2] = {cell.letter, '\0'};
    SDL_Color white = {255, 255, 255, 255};
    SDL_Surface* surface = TTF_RenderText_Solid(font, text, white);
    if (surface) {
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_Rect textRect = {x + CELL_SIZE/2 - surface->w/2, y + CELL_SIZE/2 - surface->h/2, surface->w, surface->h};
        SDL_RenderCopy(renderer, texture, nullptr, &textRect);
        SDL_FreeSurface(surface);
        SDL_DestroyTexture(texture);
    }
}

bool WordSearch::loadWordList(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    wordPool.clear();
    std::string word;
    while (std::getline(file, word)) {
        // Remove whitespace and convert to uppercase
        word.erase(std::remove_if(word.begin(), word.end(), ::isspace), word.end());
        std::transform(word.begin(), word.end(), word.begin(), ::toupper);
        
        // Only add words that fit in the grid
        if (!word.empty() && word.length() <= static_cast<size_t>(gridSize)) {
            wordPool.push_back(word);
        }
    }
    
    file.close();
    return !wordPool.empty();
}

void WordSearch::selectRandomWords(int count) {
    if (wordPool.empty()) return;
    
    // Shuffle the word pool
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(wordPool.begin(), wordPool.end(), gen);
    
    // Select up to 'count' words
    wordsToFind.clear();
    for (int i = 0; i < count && i < static_cast<int>(wordPool.size()); i++) {
        wordsToFind.push_back(wordPool[i]);
    }
}