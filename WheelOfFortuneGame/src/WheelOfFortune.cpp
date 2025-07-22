#include "WheelOfFortune.h"
#include "Game.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <random>
#include <cctype>
#include <sstream>

WheelOfFortune::WheelOfFortune(Game* game) : game(game), currentMode(GameMode::LETTER_GUESS),
                                           cursorRow(0), cursorCol(0), alphabetCursor(0),
                                           score(0), gameWon(false), gameLost(false), gameAbandoned(false) {
    displayBoard.resize(ROWS, std::vector<char>(COLS, ' '));
    playerBoard.resize(ROWS, std::vector<char>(COLS, ' '));
    LoadPuzzles("data/puzzles.ini");
}

WheelOfFortune::~WheelOfFortune() {
}

void WheelOfFortune::HandleEvent(const SDL_Event& event) {
    if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
            case SDLK_ESCAPE:
                game->SetState(GameState::START_SCREEN);
                break;
            case SDLK_s:
                if (!gameWon && !gameLost && !gameAbandoned) {
                    if (currentMode == GameMode::LETTER_GUESS) {
                        currentMode = GameMode::PUZZLE_SOLVE;
                        FindNextEmptyPosition();
                    } else {
                        currentMode = GameMode::LETTER_GUESS;
                    }
                }
                break;
            case SDLK_a:
                if (!gameWon && !gameLost && !gameAbandoned) {
                    gameAbandoned = true;
                    // Reveal all letters
                    for (char c = 'A'; c <= 'Z'; c++) {
                        guessedLetters.insert(c);
                    }
                }
                break;
            case SDLK_UP:
                if (currentMode == GameMode::LETTER_GUESS) {
                    MoveAlphabetCursor(0, -1);
                } else {
                    CyclePuzzleLetter(-1);
                }
                break;
            case SDLK_DOWN:
                if (currentMode == GameMode::LETTER_GUESS) {
                    MoveAlphabetCursor(0, 1);
                } else {
                    CyclePuzzleLetter(1);
                }
                break;
            case SDLK_LEFT:
                if (currentMode == GameMode::LETTER_GUESS) {
                    MoveAlphabetCursor(-1, 0);
                } else {
                    MovePuzzleCursor(0, -1);
                }
                break;
            case SDLK_RIGHT:
                if (currentMode == GameMode::LETTER_GUESS) {
                    MoveAlphabetCursor(1, 0);
                } else {
                    MovePuzzleCursor(0, 1);
                }
                break;
            case SDLK_RETURN:
                if (gameWon || gameLost || gameAbandoned) {
                    StartNewGame();
                } else if (currentMode == GameMode::LETTER_GUESS) {
                    char letter = 'A' + alphabetCursor;
                    GuessLetter(letter);
                } else {
                    // In solve mode, submit solution
                    if (TrySolvePuzzle()) {
                        gameWon = true;
                        score += 2000;
                    }
                }
                break;
            default:
                // Allow direct letter input in letter guess mode
                if (!gameWon && !gameLost && !gameAbandoned && currentMode == GameMode::LETTER_GUESS) {
                    char letter = static_cast<char>(event.key.keysym.sym);
                    if (letter >= 'a' && letter <= 'z') {
                        GuessLetter(toupper(letter));
                    } else if (letter >= 'A' && letter <= 'Z') {
                        GuessLetter(letter);
                    }
                }
                break;
        }
    }
}

void WheelOfFortune::Update() {
    if (!gameWon && !gameLost && !gameAbandoned) {
        if (IsPuzzleSolved()) {
            gameWon = true;
            score += 1000;
        }
    }
}

void WheelOfFortune::Render() {
    RenderBoard();
    RenderAlphabet();
    RenderUI();
}

void WheelOfFortune::StartNewGame() {
    if (puzzles.empty()) {
        std::cerr << "No puzzles loaded!" << std::endl;
        return;
    }
    
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, puzzles.size() - 1);
    
    currentPuzzle = puzzles[dis(gen)];
    guessedLetters.clear();
    currentMode = GameMode::LETTER_GUESS;
    cursorRow = 0;
    cursorCol = 0;
    alphabetCursor = 0;
    score = 0;
    gameWon = false;
    gameLost = false;
    gameAbandoned = false;
    
    InitializeBoard();
}

bool WheelOfFortune::LoadPuzzles(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Could not open puzzle file: " << filename << std::endl;
        return false;
    }
    
    puzzles.clear();
    std::string line;
    std::string currentCategory;
    
    while (std::getline(file, line)) {
        // Remove leading/trailing whitespace
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);
        
        if (line.empty()) continue;
        
        // Check if this is a category header [CATEGORY]
        if (line.front() == '[' && line.back() == ']') {
            currentCategory = line.substr(1, line.length() - 2);
        } else if (!currentCategory.empty()) {
            // This is a puzzle entry under the current category
            Puzzle puzzle;
            puzzle.category = currentCategory;
            puzzle.phrase = line;
            puzzles.push_back(puzzle);
        }
    }
    
    file.close();
    std::cout << "Loaded " << puzzles.size() << " puzzles" << std::endl;
    return !puzzles.empty();
}

void WheelOfFortune::InitializeBoard() {
    // Clear both boards
    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLS; col++) {
            displayBoard[row][col] = ' ';
            playerBoard[row][col] = ' ';
        }
    }
    
    std::vector<std::string> fittedLines = FitTextToGrid(currentPuzzle.phrase);
    
    for (int row = 0; row < ROWS && row < static_cast<int>(fittedLines.size()); row++) {
        const std::string& line = fittedLines[row];
        int startCol = (COLS - line.length()) / 2;
        
        for (int i = 0; i < static_cast<int>(line.length()) && startCol + i < COLS; i++) {
            displayBoard[row][startCol + i] = toupper(line[i]);
            playerBoard[row][startCol + i] = toupper(line[i]);
        }
    }
}

std::vector<std::string> WheelOfFortune::FitTextToGrid(const std::string& text) {
    std::vector<std::string> result;
    std::istringstream iss(text);
    std::string word;
    std::vector<std::string> words;
    
    while (iss >> word) {
        words.push_back(word);
    }
    
    std::string currentLine;
    for (const std::string& word : words) {
        if (currentLine.empty()) {
            currentLine = word;
        } else if (currentLine.length() + 1 + word.length() <= COLS) {
            currentLine += " " + word;
        } else {
            result.push_back(currentLine);
            currentLine = word;
            
            if (result.size() >= ROWS) {
                break;
            }
        }
    }
    
    if (!currentLine.empty() && result.size() < ROWS) {
        result.push_back(currentLine);
    }
    
    while (result.size() < ROWS) {
        result.push_back("");
    }
    
    return result;
}

void WheelOfFortune::MoveAlphabetCursor(int deltaCol, int deltaRow) {
    // Convert current cursor position to row/col coordinates
    int currentRow = alphabetCursor / 13;
    int currentCol = alphabetCursor % 13;
    
    // Apply the movement
    int newRow = currentRow + deltaRow;
    int newCol = currentCol + deltaCol;
    
    // Handle wrapping and bounds
    if (deltaCol != 0) {
        // Left/right movement
        newCol = std::max(0, std::min(newCol, 12));
        // If we go past the end of row 0, wrap to row 1
        if (newRow == 0 && newCol > 12) {
            newRow = 1;
            newCol = 0;
        }
        // If we go before start of row 1, wrap to row 0
        else if (newRow == 1 && newCol < 0) {
            newRow = 0;
            newCol = 12;
        }
    } else if (deltaRow != 0) {
        // Up/down movement
        newRow = std::max(0, std::min(newRow, 1));
        
        // If moving to row 1 and current column > 12, go to last position
        if (newRow == 1 && newCol > 12) {
            newCol = 12;
        }
    }
    
    // Convert back to linear position
    int newPosition = newRow * 13 + newCol;
    
    // Ensure we stay within alphabet bounds (0-25)
    alphabetCursor = std::max(0, std::min(newPosition, 25));
}

void WheelOfFortune::MovePuzzleCursor(int deltaRow, int deltaCol) {
    if (deltaCol > 0) {
        FindNextEmptyPosition();
    } else if (deltaCol < 0) {
        FindPreviousEmptyPosition();
    }
}

void WheelOfFortune::FindNextEmptyPosition() {
    int startRow = cursorRow;
    int startCol = cursorCol;
    
    do {
        cursorCol++;
        if (cursorCol >= COLS) {
            cursorCol = 0;
            cursorRow++;
            if (cursorRow >= ROWS) {
                cursorRow = 0;
            }
        }
        
        if (IsValidPosition(cursorRow, cursorCol) && 
            guessedLetters.find(displayBoard[cursorRow][cursorCol]) == guessedLetters.end()) {
            return;
        }
    } while (cursorRow != startRow || cursorCol != startCol);
}

void WheelOfFortune::FindPreviousEmptyPosition() {
    int startRow = cursorRow;
    int startCol = cursorCol;
    
    do {
        cursorCol--;
        if (cursorCol < 0) {
            cursorCol = COLS - 1;
            cursorRow--;
            if (cursorRow < 0) {
                cursorRow = ROWS - 1;
            }
        }
        
        if (IsValidPosition(cursorRow, cursorCol) && 
            guessedLetters.find(displayBoard[cursorRow][cursorCol]) == guessedLetters.end()) {
            return;
        }
    } while (cursorRow != startRow || cursorCol != startCol);
}

void WheelOfFortune::CyclePuzzleLetter(int delta) {
    if (!IsValidPosition(cursorRow, cursorCol)) return;
    
    char currentLetter = playerBoard[cursorRow][cursorCol];
    int letterIndex;
    
    if (currentLetter == ' ' || currentLetter < 'A' || currentLetter > 'Z') {
        letterIndex = (delta > 0) ? 0 : 25;
    } else {
        letterIndex = currentLetter - 'A' + delta;
        if (letterIndex < 0) letterIndex = 25;
        if (letterIndex > 25) letterIndex = 0;
    }
    
    playerBoard[cursorRow][cursorCol] = 'A' + letterIndex;
}

void WheelOfFortune::MoveCursor(int deltaRow, int deltaCol) {
    // This is now only used for legacy navigation - not needed for new system
}

void WheelOfFortune::GuessLetter(char letter) {
    if (guessedLetters.find(letter) != guessedLetters.end()) {
        return;
    }
    
    guessedLetters.insert(letter);
    
    if (IsLetterInPuzzle(letter)) {
        score += 100;
    } else {
        score = std::max(0, score - 50);
    }
}

bool WheelOfFortune::IsLetterInPuzzle(char letter) {
    std::string phrase = currentPuzzle.phrase;
    return phrase.find(toupper(letter)) != std::string::npos;
}

void WheelOfFortune::RevealLetter(char letter) {
}

bool WheelOfFortune::IsPuzzleSolved() {
    std::string phrase = currentPuzzle.phrase;
    
    for (char c : phrase) {
        if (isalpha(c) && guessedLetters.find(toupper(c)) == guessedLetters.end()) {
            return false;
        }
    }
    
    return true;
}

bool WheelOfFortune::TrySolvePuzzle() {
    if (!IsPlayerBoardComplete()) {
        return false;
    }
    
    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLS; col++) {
            if (displayBoard[row][col] != ' ' && 
                playerBoard[row][col] != displayBoard[row][col]) {
                return false;
            }
        }
    }
    
    return true;
}

bool WheelOfFortune::IsPlayerBoardComplete() {
    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLS; col++) {
            if (displayBoard[row][col] != ' ' && 
                (playerBoard[row][col] == ' ' || !isalpha(playerBoard[row][col]))) {
                return false;
            }
        }
    }
    return true;
}

char WheelOfFortune::GetCurrentLetter() {
    if (!IsValidPosition(cursorRow, cursorCol)) {
        return 0;
    }
    
    return displayBoard[cursorRow][cursorCol];
}

bool WheelOfFortune::IsValidPosition(int row, int col) {
    if (row < 0 || row >= ROWS || col < 0 || col >= COLS) {
        return false;
    }
    
    char c = displayBoard[row][col];
    return c != ' ' && isalpha(c);
}

void WheelOfFortune::RenderText(const char* text, int x, int y, SDL_Color color) {
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

void WheelOfFortune::RenderBoard() {
    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLS; col++) {
            int x = BOARD_START_X + col * (BOX_SIZE + BOX_SPACING);
            int y = BOARD_START_Y + row * (BOX_SIZE + BOX_SPACING);
            
            char letter = displayBoard[row][col];
            bool highlighted = (currentMode == GameMode::PUZZLE_SOLVE && 
                              row == cursorRow && col == cursorCol);
            bool revealed = (letter == ' ') || guessedLetters.find(letter) != guessedLetters.end();
            
            if (letter != ' ') {
                char displayLetter;
                bool shouldShow;
                
                if (currentMode == GameMode::PUZZLE_SOLVE) {
                    // In solve mode, show revealed letters OR manually entered letters
                    if (revealed) {
                        displayLetter = letter;  // Show the correct revealed letter
                        shouldShow = true;
                    } else {
                        displayLetter = playerBoard[row][col];  // Show player's guess
                        shouldShow = (displayLetter != ' ');  // Show any non-space character
                    }
                } else {
                    // In guess mode, show only revealed letters
                    displayLetter = letter;
                    shouldShow = revealed;
                }
                
                RenderLetterBox(x, y, displayLetter, highlighted, shouldShow);
            }
        }
    }
}

void WheelOfFortune::RenderAlphabet() {
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color gray = {100, 100, 100, 255};
    SDL_Color black = {0, 0, 0, 255};
    
    // Title
    // Center the alphabet title
    int centerX = Game::LOGICAL_WIDTH / 2;
    RenderText("ALPHABET:", centerX - 35, ALPHABET_START_Y - 25, white);
    
    // Render alphabet in two rows
    for (int i = 0; i < 26; i++) {
        char letter = 'A' + i;
        int row = i / 13;
        int col = i % 13;
        
        int x = ALPHABET_START_X + col * (ALPHABET_BOX_SIZE + ALPHABET_SPACING);
        int y = ALPHABET_START_Y + row * (ALPHABET_BOX_SIZE + ALPHABET_SPACING);
        
        SDL_Rect boxRect = {x, y, ALPHABET_BOX_SIZE, ALPHABET_BOX_SIZE};
        
        bool used = guessedLetters.find(letter) != guessedLetters.end();
        bool highlighted = (currentMode == GameMode::LETTER_GUESS && alphabetCursor == i);
        
        if (used) {
            SDL_SetRenderDrawColor(game->GetRenderer(), 60, 60, 60, 255);
            SDL_RenderFillRect(game->GetRenderer(), &boxRect);
            SDL_SetRenderDrawColor(game->GetRenderer(), 40, 40, 40, 255);
        } else if (highlighted) {
            SDL_SetRenderDrawColor(game->GetRenderer(), 255, 255, 0, 255);
            SDL_RenderFillRect(game->GetRenderer(), &boxRect);
            SDL_SetRenderDrawColor(game->GetRenderer(), 200, 200, 0, 255);
        } else {
            SDL_SetRenderDrawColor(game->GetRenderer(), 200, 200, 200, 255);
            SDL_RenderFillRect(game->GetRenderer(), &boxRect);
            SDL_SetRenderDrawColor(game->GetRenderer(), 160, 160, 160, 255);
        }
        
        SDL_RenderDrawRect(game->GetRenderer(), &boxRect);
        
        char letterStr[2] = {letter, '\0'};
        SDL_Color textColor = used ? gray : (highlighted ? black : black);
        
        TTF_Font* font = game->GetFont();
        if (font) {
            SDL_Surface* surface = TTF_RenderText_Blended(font, letterStr, textColor);
            if (surface) {
                SDL_Texture* texture = SDL_CreateTextureFromSurface(game->GetRenderer(), surface);
                if (texture) {
                    SDL_Rect textRect = {
                        x + (ALPHABET_BOX_SIZE - surface->w) / 2,
                        y + (ALPHABET_BOX_SIZE - surface->h) / 2,
                        surface->w,
                        surface->h
                    };
                    SDL_RenderCopy(game->GetRenderer(), texture, nullptr, &textRect);
                    SDL_DestroyTexture(texture);
                }
                SDL_FreeSurface(surface);
            }
        }
    }
}

void WheelOfFortune::RenderLetterBox(int x, int y, char letter, bool highlighted, bool revealed) {
    SDL_Renderer* renderer = game->GetRenderer();
    
    SDL_Rect boxRect = {x, y, BOX_SIZE, BOX_SIZE};
    
    if (highlighted) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
        SDL_RenderFillRect(renderer, &boxRect);
        SDL_SetRenderDrawColor(renderer, 200, 200, 0, 255);
    } else {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &boxRect);
        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    }
    
    SDL_RenderDrawRect(renderer, &boxRect);
    
    if (revealed && letter != ' ') {
        char letterStr[2] = {letter, '\0'};
        SDL_Color black = {0, 0, 0, 255};
        
        TTF_Font* font = game->GetFont();
        if (font) {
            SDL_Surface* surface = TTF_RenderText_Blended(font, letterStr, black);
            if (surface) {
                SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
                if (texture) {
                    SDL_Rect textRect = {
                        x + (BOX_SIZE - surface->w) / 2,
                        y + (BOX_SIZE - surface->h) / 2,
                        surface->w,
                        surface->h
                    };
                    SDL_RenderCopy(renderer, texture, nullptr, &textRect);
                    SDL_DestroyTexture(texture);
                }
                SDL_FreeSurface(surface);
            }
        }
    }
}

void WheelOfFortune::RenderUI() {
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color yellow = {255, 255, 0, 255};
    SDL_Color green = {0, 255, 0, 255};
    SDL_Color red = {255, 0, 0, 255};
    SDL_Color lightBlue = {173, 216, 230, 255};
    
    // Top status bar - centered
    int centerX = Game::LOGICAL_WIDTH / 2;
    
    std::string categoryText = "Category: " + currentPuzzle.category;
    RenderText(categoryText.c_str(), centerX - 90, 20, white);
    
    std::string scoreText = "Score: " + std::to_string(score);
    RenderText(scoreText.c_str(), 20, 20, white);
    
    std::string modeText = "Mode: ";
    modeText += (currentMode == GameMode::LETTER_GUESS) ? "Letter Guess" : "Puzzle Solve";
    RenderText(modeText.c_str(), Game::LOGICAL_WIDTH - 180, 20, lightBlue);
    
    if (gameWon) {
        RenderText("CONGRATULATIONS! YOU WON!", centerX - 130, 370, green);
        RenderText("Press ENTER to play again", centerX - 90, 390, yellow);
    } else if (gameLost) {
        RenderText("GAME OVER!", centerX - 45, 370, red);
        RenderText("Press ENTER to play again", centerX - 90, 390, yellow);
    } else if (gameAbandoned) {
        RenderText("PUZZLE ABANDONED - ANSWER REVEALED", centerX - 155, 370, red);
        RenderText("Press ENTER to play again", centerX - 90, 390, yellow);
    } else {
        if (currentMode == GameMode::LETTER_GUESS) {
            RenderText("ARROWS=navigate ENTER=guess S=solve A=abandon", centerX - 200, 370, white);
        } else {
            RenderText("LEFT/RIGHT=move UP/DOWN=change ENTER=submit S=back", centerX - 210, 370, white);
            
            if (!IsPlayerBoardComplete()) {
                RenderText("Fill in all letters before submitting!", centerX - 140, 390, yellow);
            }
        }
    }
    
    RenderText("ESC = menu", centerX - 40, 420, white);
}