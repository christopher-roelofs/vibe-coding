#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <vector>
#include <set>

class Game;

struct Puzzle {
    std::string phrase;
    std::string category;
};

class WheelOfFortune {
public:
    WheelOfFortune(Game* game);
    ~WheelOfFortune();
    
    void HandleEvent(const SDL_Event& event);
    void Update();
    void Render();
    
    void StartNewGame();
    bool LoadPuzzles(const std::string& filename);

private:
    Game* game;
    
    std::vector<Puzzle> puzzles;
    Puzzle currentPuzzle;
    std::vector<std::vector<char>> displayBoard;
    std::vector<std::vector<char>> playerBoard;
    std::set<char> guessedLetters;
    
    enum class GameMode {
        LETTER_GUESS,
        PUZZLE_SOLVE
    };
    
    GameMode currentMode;
    int cursorRow;
    int cursorCol;
    int alphabetCursor;
    int score;
    bool gameWon;
    bool gameLost;
    bool gameAbandoned;
    
    void InitializeBoard();
    void MoveCursor(int deltaRow, int deltaCol);
    void GuessLetter(char letter);
    bool IsLetterInPuzzle(char letter);
    void RevealLetter(char letter);
    bool IsPuzzleSolved();
    
    void RenderText(const char* text, int x, int y, SDL_Color color);
    void RenderBoard();
    void RenderUI();
    void RenderAlphabet();
    void RenderLetterBox(int x, int y, char letter, bool highlighted, bool revealed);
    
    char GetCurrentLetter();
    bool IsValidPosition(int row, int col);
    std::vector<std::string> FitTextToGrid(const std::string& text);
    
    void MoveAlphabetCursor(int deltaCol, int deltaRow);
    void MovePuzzleCursor(int deltaRow, int deltaCol);
    void CyclePuzzleLetter(int delta);
    bool TrySolvePuzzle();
    bool IsPlayerBoardComplete();
    void FindNextEmptyPosition();
    void FindPreviousEmptyPosition();
    
    static const int ROWS = 4;
    static const int COLS = 14;
    static const int BOX_SIZE = 25;
    static const int BOX_SPACING = 3;
    static const int BOARD_START_X = 125;  // Centered for 640px width
    static const int BOARD_START_Y = 80;
    static const int ALPHABET_START_X = 178; // Centered for 640px width
    static const int ALPHABET_START_Y = 240;
    static const int ALPHABET_BOX_SIZE = 20;
    static const int ALPHABET_SPACING = 2;
};