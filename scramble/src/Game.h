#ifndef GAME_H
#define GAME_H

#include <SDL.h>
#include <SDL_ttf.h>
#include <vector>
#include <string>
#include <memory>
#include "Settings.h"
#include "WordList.h"

enum class GameState {
    START_SCREEN,
    GAME_SCREEN,
    RESULTS_SCREEN // For success/failure messages
};

class Game {
public:
    Game();
    ~Game();

    bool init();
    void run();

private:
    void processInput();
    void update();
    void render();

    void cleanup();

    // Helper functions
    void loadWordLists();
    std::string scrambleWord(const std::string& word);
    void renderText(const std::string& text, int x, int y, SDL_Color color, bool centered = false, TTF_Font* customFont = nullptr, int wrapWidth = 0);
    void prepareNewWord();

    // SDL specific
    SDL_Window* window_;
    SDL_Renderer* renderer_;
    TTF_Font* font_;
    TTF_Font* font_scrambled_word_;
    TTF_Font* font_list_name_title_;

    Settings settings_;
    std::vector<WordList> available_word_lists_;
    size_t current_word_list_index_;

    std::vector<size_t> shuffled_indices_for_current_list_;
    size_t current_shuffled_idx_position_;

    GameState current_state_;
    bool running_;

    // Game screen specific
    WordEntry current_word_entry_;
    std::string scrambled_display_word_; // The word displayed with mixed up letters
    std::string current_guess_;
    size_t selected_char_index_scrambled_; // Index in scrambled_display_word_ for selection cursor
    
    std::string game_message_;
    std::string m_currentHintText; // Stores the hint for the current word
    bool m_showHint; // Flag to control hint visibility
    Uint32 message_display_start_time_;
    const Uint32 MESSAGE_DISPLAY_DURATION = 3000; // 3 seconds

    // Colors - loaded from settings
    SDL_Color color_background_;
    SDL_Color color_title_text_;
    SDL_Color color_list_name_text_;
    SDL_Color color_list_meta_text_;
    SDL_Color color_scrambled_word_text_;
    SDL_Color color_scrambled_word_cursor_text_;
    SDL_Color color_guessed_word_text_;
    SDL_Color color_message_text_;
    SDL_Color color_help_text_;
};

#endif // GAME_H
