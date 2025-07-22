#ifndef WORDSEARCH_H
#define WORDSEARCH_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <vector>
#include <string>
#include <utility>
#include <set>
#include <map>

struct Cell {
    char letter;
    bool selected;
    bool found;
};

class WordSearch {
public:
    WordSearch(SDL_Renderer* renderer, TTF_Font* font, int gridSize = 15);
    ~WordSearch();
    
    void handleInput(SDL_Event& event);
    void update();
    void render();
    
    void newGame();
    void newGame(const std::string& theme);
    bool isGameComplete() const;
    bool wantsToQuit() const { return quit; }
    
private:
    SDL_Renderer* renderer;
    TTF_Font* font;
    
    static const int CELL_SIZE = 30;
    int gridSize;
    std::vector<std::vector<Cell>> grid;
    std::vector<std::string> wordsToFind;
    std::vector<std::string> foundWords;
    std::vector<std::string> wordPool;
    std::map<std::string, std::vector<std::string>> themes;
    std::string currentTheme;
    
    bool loadWordList(const std::string& filename);
    bool loadThemes(const std::string& filename);
    void selectRandomTheme();
    void selectRandomWords(int count);
    
    int cursorX, cursorY;
    bool selecting;
    int selectStartX, selectStartY;
    bool quit;
    
    void generateGrid();
    void placeWord(const std::string& word);
    void fillEmptyCells();
    void checkSelection();
    std::string getSelectedWord();
    void renderGrid();
    void renderWordList();
    void renderCell(int x, int y, const Cell& cell);
};

#endif