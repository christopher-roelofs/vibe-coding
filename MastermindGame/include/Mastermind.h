#ifndef MASTERMIND_H
#define MASTERMIND_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <vector>
#include <array>

enum class PegColor {
    EMPTY = 0,
    RED,
    GREEN,
    BLUE,
    YELLOW,
    ORANGE,
    PURPLE,
    PINK,
    COUNT
};

struct Guess {
    std::vector<PegColor> colors;
    int blackPegs;  // Correct color and position
    int whitePegs;  // Correct color, wrong position
};

class Mastermind {
public:
    Mastermind(SDL_Renderer* renderer, TTF_Font* font, TTF_Font* smallFont, int codeLength, int numColors);
    ~Mastermind();
    
    void handleInput(SDL_Event& event);
    void update();
    void render();
    
    void newGame();
    bool isGameWon() const { return gameWon; }
    bool isGameLost() const { return gameLost; }
    bool wantsToQuit() const { return quit; }
    
private:
    SDL_Renderer* renderer;
    TTF_Font* font;
    TTF_Font* smallFont;
    
    static const int MAX_GUESSES = 12;
    static const int PEG_SIZE = 32;    // Reduced from 40 to 32
    static const int SMALL_PEG_SIZE = 12;
    
    int codeLength;
    int numColors;
    
    std::vector<PegColor> secretCode;
    std::vector<Guess> guesses;
    std::vector<PegColor> currentGuess;
    int currentPegIndex;
    
    bool gameWon;
    bool gameLost;
    bool quit;
    bool showCode;  // For debugging or after game ends
    
    void generateSecretCode();
    void submitGuess();
    void calculateFeedback(Guess& guess);
    
    void renderBoard();
    void renderGuess(const Guess& guess, int row);
    void renderCurrentGuess();
    void renderInstructions();
    void renderPeg(int x, int y, PegColor color, int size = PEG_SIZE);
    void renderFeedback(int x, int y, int blackPegs, int whitePegs);
    
    SDL_Color getPegColor(PegColor peg);
    PegColor getColorFromKey(SDL_Keycode key);
    
    int boardX;
    int boardY;
    int paletteX;
    int paletteY;
};

#endif