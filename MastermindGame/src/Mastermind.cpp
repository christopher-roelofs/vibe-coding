#include "../include/Mastermind.h"
#include <random>
#include <algorithm>
#include <ctime>

Mastermind::Mastermind(SDL_Renderer* renderer, TTF_Font* font, TTF_Font* smallFont, int codeLength, int numColors)
    : renderer(renderer), font(font), smallFont(smallFont), codeLength(codeLength), numColors(numColors),
      currentPegIndex(0), gameWon(false), gameLost(false), quit(false), showCode(false) {
    
    boardX = 20;  // Smaller margin for 640x480
    boardY = 30;  // Smaller margin for 640x480
    
    secretCode.resize(codeLength);
    currentGuess.resize(codeLength);
    
    for (auto& peg : currentGuess) {
        peg = PegColor::EMPTY;
    }
}

Mastermind::~Mastermind() {
}

void Mastermind::handleInput(SDL_Event& event) {
    if (event.type == SDL_KEYDOWN) {
        if (gameWon || gameLost) return;
        
        switch (event.key.keysym.sym) {
            case SDLK_UP:
                if (currentPegIndex > 0) currentPegIndex--;
                break;
                
            case SDLK_DOWN:
                if (currentPegIndex < codeLength - 1) currentPegIndex++;
                break;
                
            case SDLK_LEFT:
                {
                    int currentColorIndex = static_cast<int>(currentGuess[currentPegIndex]);
                    if (currentColorIndex == 0) {
                        currentGuess[currentPegIndex] = static_cast<PegColor>(numColors);
                    } else {
                        currentGuess[currentPegIndex] = static_cast<PegColor>(currentColorIndex - 1);
                    }
                }
                break;
                
            case SDLK_RIGHT:
                {
                    int currentColorIndex = static_cast<int>(currentGuess[currentPegIndex]);
                    if (currentColorIndex >= numColors) {
                        currentGuess[currentPegIndex] = PegColor::EMPTY;
                    } else {
                        currentGuess[currentPegIndex] = static_cast<PegColor>(currentColorIndex + 1);
                    }
                }
                break;
                
            case SDLK_RETURN:
                submitGuess();
                break;
                
            case SDLK_BACKSPACE:
                currentGuess[currentPegIndex] = PegColor::EMPTY;
                break;
                
            case SDLK_ESCAPE:
                quit = true;
                break;
                
            case SDLK_s:
                showCode = !showCode;  // Toggle secret code visibility for debugging
                break;
        }
    }
}

void Mastermind::update() {
}

void Mastermind::render() {
    renderBoard();
    renderInstructions();
    
    // Render previous guesses
    for (size_t i = 0; i < guesses.size(); i++) {
        renderGuess(guesses[i], i);
    }
    
    // Render current guess
    if (!gameWon && !gameLost) {
        renderCurrentGuess();
    }
    
    // Show secret code if game is over or debug mode
    if (showCode || gameWon || gameLost) {
        int secretX = boardX + MAX_GUESSES * (PEG_SIZE + 15) + 20;
        
        // Display secret code vertically at the end (no text label)
        for (int i = 0; i < codeLength; i++) {
            int secretY = boardY + i * (PEG_SIZE + 10) + 5;
            renderPeg(secretX, secretY, secretCode[i], 28);
        }
    }
}

void Mastermind::newGame() {
    guesses.clear();
    currentPegIndex = 0;
    gameWon = false;
    gameLost = false;
    showCode = false;
    
    for (auto& peg : currentGuess) {
        peg = PegColor::EMPTY;
    }
    
    generateSecretCode();
}

void Mastermind::generateSecretCode() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, numColors);
    
    for (auto& peg : secretCode) {
        peg = static_cast<PegColor>(dis(gen));
    }
}

void Mastermind::submitGuess() {
    // Check if all pegs are filled
    for (const auto& peg : currentGuess) {
        if (peg == PegColor::EMPTY) {
            return;
        }
    }
    
    Guess guess;
    guess.colors = currentGuess;
    calculateFeedback(guess);
    guesses.push_back(guess);
    
    // Check win condition
    if (guess.blackPegs == codeLength) {
        gameWon = true;
    } else if (guesses.size() >= MAX_GUESSES) {
        gameLost = true;
    }
    
    // Reset current guess
    for (auto& peg : currentGuess) {
        peg = PegColor::EMPTY;
    }
    currentPegIndex = 0;
}

void Mastermind::calculateFeedback(Guess& guess) {
    guess.blackPegs = 0;
    guess.whitePegs = 0;
    
    std::vector<bool> secretUsed(codeLength, false);
    std::vector<bool> guessUsed(codeLength, false);
    
    // First pass: count black pegs (correct color and position)
    for (int i = 0; i < codeLength; i++) {
        if (guess.colors[i] == secretCode[i]) {
            guess.blackPegs++;
            secretUsed[i] = true;
            guessUsed[i] = true;
        }
    }
    
    // Second pass: count white pegs (correct color, wrong position)
    for (int i = 0; i < codeLength; i++) {
        if (!guessUsed[i]) {
            for (int j = 0; j < codeLength; j++) {
                if (!secretUsed[j] && guess.colors[i] == secretCode[j]) {
                    guess.whitePegs++;
                    secretUsed[j] = true;
                    break;
                }
            }
        }
    }
}

void Mastermind::renderBoard() {
    // Calculate board dimensions for horizontal layout
    int pegSpacing = 10;  // Vertical spacing between pegs in a column (more space for underlines)
    int colSpacing = 15;  // Horizontal spacing between guess columns (very compact)
    
    // Board is now: MAX_GUESSES columns wide, codeLength pegs tall
    int boardWidth = MAX_GUESSES * (PEG_SIZE + colSpacing) + 20;
    int boardHeight = codeLength * (PEG_SIZE + pegSpacing) + 20;
    
    // Draw board background
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
    SDL_Rect boardRect = {boardX - 5, boardY - 5, boardWidth, boardHeight};
    SDL_RenderFillRect(renderer, &boardRect);
    
    // Draw vertical grid lines (separating guess columns)
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    for (int i = 0; i <= MAX_GUESSES; i++) {
        int x = boardX + i * (PEG_SIZE + colSpacing);
        SDL_RenderDrawLine(renderer, x, boardY, x, boardY + codeLength * (PEG_SIZE + pegSpacing));
    }
}

void Mastermind::renderGuess(const Guess& guess, int col) {
    int pegSpacing = 10;  // Vertical spacing between pegs in a column (more space for underlines)
    int colSpacing = 15;  // Horizontal spacing between guess columns (very compact)
    
    // Column position (was row position)
    int x = boardX + col * (PEG_SIZE + colSpacing) + 5;
    
    // Render colored pegs vertically with underline feedback
    for (int i = 0; i < codeLength; i++) {
        int y = boardY + i * (PEG_SIZE + pegSpacing) + 5;
        renderPeg(x, y, guess.colors[i]);
        
        // Calculate which position gets which feedback
        bool hasBlackPeg = false;
        bool hasWhitePeg = false;
        
        // Check if this position has correct color and position (black peg)
        if (guess.colors[i] == secretCode[i]) {
            hasBlackPeg = true;
        } else {
            // Check if this color exists elsewhere in the secret (white peg)
            // Only count as white if we haven't used up all instances
            int colorCountInSecret = 0;
            int colorCountInGuessAtCorrectPos = 0;
            int colorCountInGuessBeforeThisPos = 0;
            
            // Count how many times this color appears in secret
            for (int j = 0; j < codeLength; j++) {
                if (secretCode[j] == guess.colors[i]) {
                    colorCountInSecret++;
                }
            }
            
            // Count how many times we've already placed this color correctly
            for (int j = 0; j < codeLength; j++) {
                if (guess.colors[j] == secretCode[j] && guess.colors[j] == guess.colors[i]) {
                    colorCountInGuessAtCorrectPos++;
                }
            }
            
            // Count how many times this color appeared before this position
            for (int j = 0; j < i; j++) {
                if (guess.colors[j] == guess.colors[i] && guess.colors[j] != secretCode[j]) {
                    colorCountInGuessBeforeThisPos++;
                }
            }
            
            // This position gets a white peg if there are still instances available
            if (colorCountInSecret > colorCountInGuessAtCorrectPos + colorCountInGuessBeforeThisPos) {
                hasWhitePeg = true;
            }
        }
        
        // Draw underline feedback beneath each peg
        int underlineY = y + PEG_SIZE + 2;
        int underlineWidth = PEG_SIZE;
        int underlineHeight = 3;
        
        if (hasBlackPeg) {
            // Black underline for correct position
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_Rect rect = {x, underlineY, underlineWidth, underlineHeight};
            SDL_RenderFillRect(renderer, &rect);
        } else if (hasWhitePeg) {
            // White underline for correct color, wrong position
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_Rect rect = {x, underlineY, underlineWidth, underlineHeight};
            SDL_RenderFillRect(renderer, &rect);
            // Add black border for visibility
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderDrawRect(renderer, &rect);
        }
    }
}

void Mastermind::renderCurrentGuess() {
    int pegSpacing = 10;  // Vertical spacing between pegs in a column (more space for underlines)
    int colSpacing = 15;  // Horizontal spacing between guess columns (very compact)
    
    // Current guess column position
    int x = boardX + guesses.size() * (PEG_SIZE + colSpacing) + 5;
    
    for (int i = 0; i < codeLength; i++) {
        int y = boardY + i * (PEG_SIZE + pegSpacing) + 5;
        
        // Highlight current peg position
        if (i == currentPegIndex) {
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
            SDL_Rect highlight = {x - 3, y - 3, PEG_SIZE + 6, PEG_SIZE + 6};
            SDL_RenderDrawRect(renderer, &highlight);
        }
        
        renderPeg(x, y, currentGuess[i]);
    }
}

void Mastermind::renderInstructions() {
    if (!smallFont) return;  // Safety check
    
    SDL_Color gray = {180, 180, 180, 255};
    
    // Split instructions into two lines
    const char* line1 = "Up/Down: Move position | Left/Right: Change color | Enter: Submit | Backspace: Clear";
    const char* line2 = "Black/White underlines show feedback | ESC: Menu";
    
    // First line
    SDL_Surface* surface1 = TTF_RenderText_Solid(smallFont, line1, gray);
    if (surface1) {
        SDL_Texture* texture1 = SDL_CreateTextureFromSurface(renderer, surface1);
        if (texture1) {
            SDL_Rect rect1 = {320 - surface1->w/2, 440, surface1->w, surface1->h};
            SDL_RenderCopy(renderer, texture1, nullptr, &rect1);
            SDL_DestroyTexture(texture1);
        }
        SDL_FreeSurface(surface1);
    }
    
    // Second line
    SDL_Surface* surface2 = TTF_RenderText_Solid(smallFont, line2, gray);
    if (surface2) {
        SDL_Texture* texture2 = SDL_CreateTextureFromSurface(renderer, surface2);
        if (texture2) {
            SDL_Rect rect2 = {320 - surface2->w/2, 460, surface2->w, surface2->h};
            SDL_RenderCopy(renderer, texture2, nullptr, &rect2);
            SDL_DestroyTexture(texture2);
        }
        SDL_FreeSurface(surface2);
    }
}

void Mastermind::renderPeg(int x, int y, PegColor color, int size) {
    if (color == PegColor::EMPTY) {
        SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);
        SDL_Rect rect = {x, y, size, size};
        SDL_RenderDrawRect(renderer, &rect);
    } else {
        SDL_Color c = getPegColor(color);
        SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
        
        // Draw filled circle (approximated with rectangles)
        int radius = size / 2;
        for (int w = 0; w < radius * 2; w++) {
            for (int h = 0; h < radius * 2; h++) {
                int dx = radius - w;
                int dy = radius - h;
                if ((dx*dx + dy*dy) <= (radius * radius)) {
                    SDL_RenderDrawPoint(renderer, x + w, y + h);
                }
            }
        }
        
        // Add number overlay for accessibility
        if (smallFont) {
            std::string numberText = std::to_string(static_cast<int>(color));
            SDL_Color textColor = {255, 255, 255, 255}; // White text
            
            // Use black text for light colors (yellow)
            if (color == PegColor::YELLOW) {
                textColor = {0, 0, 0, 255};
            }
            
            SDL_Surface* surface = TTF_RenderText_Solid(smallFont, numberText.c_str(), textColor);
            if (surface) {
                SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
                if (texture) {
                    // Center the number in the peg
                    int textX = x + (size - surface->w) / 2;
                    int textY = y + (size - surface->h) / 2;
                    SDL_Rect rect = {textX, textY, surface->w, surface->h};
                    SDL_RenderCopy(renderer, texture, nullptr, &rect);
                    SDL_DestroyTexture(texture);
                }
                SDL_FreeSurface(surface);
            }
        }
    }
}

void Mastermind::renderFeedback(int x, int y, int blackPegs, int whitePegs) {
    int pegCount = 0;
    
    // Render black pegs (correct position)
    for (int i = 0; i < blackPegs; i++) {
        int px = x + pegCount * (SMALL_PEG_SIZE + 3);
        int py = y + (PEG_SIZE - SMALL_PEG_SIZE) / 2; // Center vertically with main pegs
        
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_Rect rect = {px, py, SMALL_PEG_SIZE, SMALL_PEG_SIZE};
        SDL_RenderFillRect(renderer, &rect);
        
        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
        SDL_RenderDrawRect(renderer, &rect);
        
        pegCount++;
    }
    
    // Render white pegs (wrong position)
    for (int i = 0; i < whitePegs; i++) {
        int px = x + pegCount * (SMALL_PEG_SIZE + 3);
        int py = y + (PEG_SIZE - SMALL_PEG_SIZE) / 2; // Center vertically with main pegs
        
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_Rect rect = {px, py, SMALL_PEG_SIZE, SMALL_PEG_SIZE};
        SDL_RenderFillRect(renderer, &rect);
        
        SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
        SDL_RenderDrawRect(renderer, &rect);
        
        pegCount++;
    }
}

SDL_Color Mastermind::getPegColor(PegColor peg) {
    switch (peg) {
        case PegColor::RED:    return {255, 0, 0, 255};
        case PegColor::GREEN:  return {0, 255, 0, 255};
        case PegColor::BLUE:   return {0, 0, 255, 255};
        case PegColor::YELLOW: return {255, 255, 0, 255};
        case PegColor::ORANGE: return {255, 165, 0, 255};
        case PegColor::PURPLE: return {128, 0, 128, 255};
        default:               return {80, 80, 80, 255};
    }
}

PegColor Mastermind::getColorFromKey(SDL_Keycode key) {
    switch (key) {
        case SDLK_1: return PegColor::RED;
        case SDLK_2: return PegColor::GREEN;
        case SDLK_3: return PegColor::BLUE;
        case SDLK_4: return PegColor::YELLOW;
        case SDLK_5: return PegColor::ORANGE;
        case SDLK_6: return PegColor::PURPLE;
        default:     return PegColor::EMPTY;
    }
}