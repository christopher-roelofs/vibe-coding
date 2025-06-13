#include "Game.h"
#include <iostream>
#include <filesystem> // For C++17 directory iteration
#include <algorithm> // For std::shuffle
#include <random>    // For std::mt19937 and std::random_device
#include <vector>
#include <numeric> // For std::iota

const int DEFAULT_FONT_SIZE = 24;

// For C++17, otherwise use experimental or a third-party library
namespace fs = std::filesystem;

Game::Game() :
    window_(nullptr),
    renderer_(nullptr),
    font_(nullptr),
    font_scrambled_word_(nullptr),
    font_list_name_title_(nullptr),
    current_word_list_index_(0),

    current_shuffled_idx_position_(0),
    current_state_(GameState::START_SCREEN),
    running_(false),
    selected_char_index_scrambled_(0),
    game_message_(""),
    m_currentHintText(""),
    m_showHint(false),
    message_display_start_time_(0)
{}

Game::~Game() {
    cleanup();
}

bool Game::init() {
    if (!settings_.load("settings.ini")) {
        std::cerr << "Failed to load settings.ini. Using default values." << std::endl;
        // Continue with defaults or handle error as preferred
    }

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // Initialize SDL_ttf
    if (TTF_Init() == -1) {
        std::cerr << "SDL_ttf could not initialize! TTF_Error: " << TTF_GetError() << std::endl;
        SDL_Quit();
        return false;
    }

    // Create window
    window_ = SDL_CreateWindow("Word Scramble", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                               640, 480, SDL_WINDOW_SHOWN); // Fixed size for now
    if (!window_) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        TTF_Quit();
        SDL_Quit();
        return false;
    }

    // Create renderer
    renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer_) {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window_);
        TTF_Quit();
        SDL_Quit();
        return false;
    }

    // Load font
    std::string font_filename = settings_.getFontPath();
    std::string full_font_path = "assets/fonts/" + font_filename;

    font_ = TTF_OpenFont(full_font_path.c_str(), DEFAULT_FONT_SIZE);
    if (!font_) {
        std::cerr << "Failed to load font: " << full_font_path << "! TTF_Error: " << TTF_GetError() << std::endl;
        // Fallback or error handling
    }

    // Load specialized fonts
    int scrambled_font_size = static_cast<int>(DEFAULT_FONT_SIZE * 2.0);
    font_scrambled_word_ = TTF_OpenFont(full_font_path.c_str(), scrambled_font_size);
    if (!font_scrambled_word_) {
        std::cerr << "Failed to load large font for scrambled word: " << full_font_path << " at size " << scrambled_font_size << "! TTF_Error: " << TTF_GetError() << std::endl;
    }

    int list_name_font_size = static_cast<int>(DEFAULT_FONT_SIZE * 1.2);
    font_list_name_title_ = TTF_OpenFont(full_font_path.c_str(), list_name_font_size);
    if (!font_list_name_title_) {
        std::cerr << "Failed to load list name title font! SDL_ttf Error: " << TTF_GetError() << std::endl;
        font_list_name_title_ = font_; // Fallback to default font
    }

    // Load colors from settings
    color_background_ = settings_.getColor("background", {32, 32, 32, 255});
    color_title_text_ = settings_.getColor("title_text", {255, 255, 160, 255});
    color_list_name_text_ = settings_.getColor("list_name_text", {255, 255, 255, 255});
    color_list_meta_text_ = settings_.getColor("list_meta_text", {204, 204, 204, 255});
    color_scrambled_word_text_ = settings_.getColor("scrambled_word_text", {255, 255, 0, 255});
    color_scrambled_word_cursor_text_ = settings_.getColor("scrambled_word_cursor_text", {255, 0, 255, 255});
    color_guessed_word_text_ = settings_.getColor("guessed_word_text", {0, 255, 0, 255});
    color_message_text_ = settings_.getColor("message_text", {255, 255, 255, 255});
    color_help_text_ = settings_.getColor("help_text", {160, 160, 160, 255});

    loadWordLists();
    if (available_word_lists_.empty()) {
        std::cerr << "No word lists found or loaded. The game cannot start." << std::endl;
        // You might want to display an error message on screen too
        game_message_ = "Error: No word lists found!";
        // return false; // Or allow to run and show the error
    }

    running_ = true;
    return true;
}

void Game::loadWordLists() {
    std::string path = "assets/word_lists";
    try {
        for (const auto& entry : fs::directory_iterator(path)) {
            if (entry.is_regular_file() && entry.path().extension() == ".json") {
                WordList wl;
                if (WordList::loadFromFile(entry.path().string(), wl)) {
                    if (!wl.words.empty()) {
                         available_word_lists_.push_back(wl);
                    } else {
                        std::cerr << "Word list " << entry.path().filename() << " is empty or failed to load words." << std::endl;
                    }
                } else {
                    std::cerr << "Failed to load word list: " << entry.path().string() << std::endl;
                }
            }
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error while trying to load word lists: " << e.what() << std::endl;
    }
    if (available_word_lists_.empty()) {
        std::cout << "No word lists were loaded. Please check 'assets/word_lists' directory and JSON files." << std::endl;
    }
}

std::string Game::scrambleWord(const std::string& word) {
    if (word.empty()) return "";
    std::string scrambled = word;
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(scrambled.begin(), scrambled.end(), g);
    // Ensure it's not the same as original, unless it's a very short word or all same letters
    if (word.length() > 1 && scrambled == word) {
        // Try one more shuffle
        std::shuffle(scrambled.begin(), scrambled.end(), g);
        if (scrambled == word) { // If still same, swap first two if different
            if (word.length() > 1 && scrambled[0] != scrambled[1]) {
                 std::swap(scrambled[0], scrambled[1]);
            }
        }
    }
    return scrambled;
}

void Game::prepareNewWord() {
        if (available_word_lists_.empty() || current_word_list_index_ < 0 || static_cast<size_t>(current_word_list_index_) >= available_word_lists_.size()) {
        game_message_ = "Error: No word list selected or available.";
        current_state_ = GameState::START_SCREEN; // Go back if something is wrong
        return;
    }

    const auto& current_list = available_word_lists_[current_word_list_index_];
    if (current_list.words.empty()) {
        game_message_ = "Error: Selected word list is empty.";
        current_state_ = GameState::START_SCREEN;
        return;
    }

    if (shuffled_indices_for_current_list_.empty() || current_shuffled_idx_position_ >= shuffled_indices_for_current_list_.size()) {
        game_message_ = "Word list complete! Press Enter.";
        current_state_ = GameState::RESULTS_SCREEN;
        message_display_start_time_ = SDL_GetTicks();
        return;
    }

    size_t actual_word_index = shuffled_indices_for_current_list_[current_shuffled_idx_position_];
    current_word_entry_ = current_list.words[actual_word_index];
    current_shuffled_idx_position_++;

    if (current_word_entry_.word.empty()) {
        game_message_ = "Error: Loaded empty word. Skipping.";
        // Attempt to load the next word immediately or handle error
        prepareNewWord(); // This could lead to recursion if all words are empty
        return;
    }

    scrambled_display_word_ = scrambleWord(current_word_entry_.word); 
    current_guess_ = "";
    selected_char_index_scrambled_ = 0;
    game_message_ = ""; // Clear any previous game message
    m_currentHintText = current_word_entry_.hint;
    m_showHint = false;
}


void Game::run() {
    while (running_) {
        processInput();
        update();
        render();
    }
}

void Game::processInput() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            running_ = false;
        }
        if (event.type == SDL_KEYDOWN) {
            // Global ESC handler
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                if (current_state_ == GameState::GAME_SCREEN || current_state_ == GameState::RESULTS_SCREEN) {
                    current_state_ = GameState::START_SCREEN;
                    game_message_ = ""; // Clear any messages
                    m_showHint = false; // Also hide hint
                } else { // START_SCREEN
                    running_ = false;
                }
                continue; // Skip further processing for this key event
            }

            // State-specific key handlers
            switch (current_state_) {
                case GameState::START_SCREEN: {
                    if (available_word_lists_.empty()) {
                        if (event.key.keysym.sym == SDLK_RETURN) running_ = false;
                        break;
                    }
                    if (event.key.keysym.sym == SDLK_RIGHT) {
                        current_word_list_index_ = (current_word_list_index_ + 1) % available_word_lists_.size();
                    } else if (event.key.keysym.sym == SDLK_LEFT) {
                        current_word_list_index_ = (current_word_list_index_ - 1 + available_word_lists_.size()) % available_word_lists_.size();
                    } else if (event.key.keysym.sym == SDLK_RETURN) {
                        const auto& current_list = available_word_lists_[current_word_list_index_];
                        shuffled_indices_for_current_list_.clear();
                        if (!current_list.words.empty()) {
                            shuffled_indices_for_current_list_.resize(current_list.words.size());
                            std::iota(shuffled_indices_for_current_list_.begin(), shuffled_indices_for_current_list_.end(), 0);
                            std::random_device rd;
                            std::mt19937 g(rd());
                            std::shuffle(shuffled_indices_for_current_list_.begin(), shuffled_indices_for_current_list_.end(), g);
                        }
                        current_shuffled_idx_position_ = 0;
                        prepareNewWord();
                        if (!current_word_entry_.word.empty()) {
                            current_state_ = GameState::GAME_SCREEN;
                        }
                    }
                    break;
                }
                case GameState::GAME_SCREEN: {
                    if (event.key.keysym.sym == SDLK_RIGHT) {
                        if (!scrambled_display_word_.empty()) {
                            selected_char_index_scrambled_ = (selected_char_index_scrambled_ + 1) % scrambled_display_word_.length();
                        }
                    } else if (event.key.keysym.sym == SDLK_LEFT) {
                        if (!scrambled_display_word_.empty()) {
                            selected_char_index_scrambled_ = (selected_char_index_scrambled_ == 0) ? scrambled_display_word_.length() - 1 : selected_char_index_scrambled_ - 1;
                        }
                    } else if (event.key.keysym.sym == SDLK_RETURN) {
                        if (current_guess_.length() < current_word_entry_.word.length()) {
                            current_guess_ += scrambled_display_word_[selected_char_index_scrambled_];
                        }
                    } else if (event.key.keysym.sym == SDLK_BACKSPACE || event.key.keysym.sym == SDLK_DELETE) {
                        if (!current_guess_.empty()) {
                            current_guess_.pop_back();
                        }
                    } else if (event.key.keysym.sym == SDLK_SPACE) {
                        if (current_guess_.length() == current_word_entry_.word.length()) {
                            if (current_guess_ == current_word_entry_.word) {
                                game_message_ = "Correct! Press Enter for next word.";
                                // Hint is now shown with 'H' key
                                current_state_ = GameState::RESULTS_SCREEN;
                                message_display_start_time_ = SDL_GetTicks();
                            } else {
                                game_message_ = "Incorrect. Try again.";
                                message_display_start_time_ = SDL_GetTicks();
                            }
                        }
                    } else if (event.key.keysym.sym == SDLK_h) {
                        m_showHint = !m_showHint;
                    }
                    break;
                }
                case GameState::RESULTS_SCREEN: {
                    if (event.key.keysym.sym == SDLK_RETURN) {
                        prepareNewWord();
                        if (!current_word_entry_.word.empty()) {
                            current_state_ = GameState::GAME_SCREEN;
                            game_message_ = "";
                        } else {
                            current_state_ = GameState::START_SCREEN;
                            game_message_ = "";
                        }
                    }
                    break;
                }
            }
        }
    }
}


void Game::update() {
    // Currently, update doesn't do much beyond what's handled by input and render logic
    // for state transitions. If there were animations or timed events not directly
    // tied to input, they would go here.
}

void Game::renderText(const std::string& text, int x, int y, SDL_Color color, bool centered, TTF_Font* customFont, int wrapWidth) {
    if (!font_ && !customFont) {
        // std::cerr << "RenderText called but no font is loaded!" << std::endl;
        return; // Cannot render text without a font
    }
    TTF_Font* fontToUse = customFont ? customFont : font_;
    if (!fontToUse) return;

    SDL_Surface* surface = nullptr;
    if (wrapWidth > 0) {
        surface = TTF_RenderText_Blended_Wrapped(fontToUse, text.c_str(), color, wrapWidth);
    } else {
        surface = TTF_RenderText_Blended(fontToUse, text.c_str(), color);
    }

    if (!surface) {
        std::cerr << "Unable to render text surface! TTF_Error: " << TTF_GetError() << std::endl;
        return;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer_, surface);
    if (!texture) {
        std::cerr << "Unable to create texture from rendered text! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_FreeSurface(surface);
        return;
    }

    SDL_Rect destRect;
    destRect.w = surface->w;
    destRect.h = surface->h;
    if (centered) {
        destRect.x = x - surface->w / 2;
        destRect.y = y - surface->h / 2;
    } else {
        destRect.x = x;
        destRect.y = y;
    }

    SDL_RenderCopy(renderer_, texture, nullptr, &destRect);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

void Game::render() {
    SDL_SetRenderDrawColor(renderer_, color_background_.r, color_background_.g, color_background_.b, color_background_.a);
    SDL_RenderClear(renderer_);

    if (current_state_ == GameState::START_SCREEN) {
        renderText("Word Scramble", 320, 50, color_title_text_, true);
        if (!available_word_lists_.empty()) {
            const auto& current_list = available_word_lists_[current_word_list_index_];
            renderText(current_list.name, 320, 150, color_list_name_text_, true);
            renderText("Author: " + current_list.author, 320, 190, color_list_meta_text_, true);
            renderText("Date: " + current_list.date, 320, 220, color_list_meta_text_, true);
            renderText(current_list.description, 320, 260, color_list_meta_text_, true, nullptr, 580); // Wrap description
            renderText("< Left/Right to Change | Enter to Select >", 320, 400, color_help_text_, true);
        } else {
            renderText("No word lists found.", 320, 200, color_message_text_, true);
            renderText("Please add .json files to assets/word_lists/", 320, 240, color_message_text_, true, nullptr, 600);
            renderText("Press Enter to Exit", 320, 300, color_help_text_, true);
        }
         if (!game_message_.empty()) { // Display general messages like errors on start screen
            renderText(game_message_, 320, 350, color_message_text_, true, nullptr, 600);
        }
    } else if (current_state_ == GameState::GAME_SCREEN || current_state_ == GameState::RESULTS_SCREEN) {
        // Render the current list name
        if (current_word_list_index_ < available_word_lists_.size()) {
            const auto& current_list = available_word_lists_[current_word_list_index_];
            renderText(current_list.name, 320, 60, color_list_name_text_, true, font_list_name_title_);
        }

        // Display Scrambled Word with cursor
        // Render scrambled word (larger and higher)
        if (font_scrambled_word_ && !scrambled_display_word_.empty()) {
            int char_advance_scrambled;
            // For monospaced fonts, width of 'M' is a good representative for character advance.
            if (TTF_SizeText(font_scrambled_word_, "M", &char_advance_scrambled, nullptr) != 0 || char_advance_scrambled <= 0) {
                // Fallback: estimate advance based on 1.5x default font size's scaled spacing logic.
                char_advance_scrambled = static_cast<int>((DEFAULT_FONT_SIZE * 1.5 / 2.0) + (4.0 * 1.5) ); // Approx (36/2 + 6) = 24, using DEFAULT_FONT_SIZE
                if (char_advance_scrambled <= 0) char_advance_scrambled = static_cast<int>(DEFAULT_FONT_SIZE * 1.5 * 0.66); // Adjusted fallback based on DEFAULT_FONT_SIZE
            }

            int total_text_width = scrambled_display_word_.length() * char_advance_scrambled;
            int text_render_x_start = 320 - total_text_width / 2; // Centering
            int text_render_y = 100; // New Y position (higher)

            for (size_t i = 0; i < scrambled_display_word_.length(); ++i) {
                std::string char_str(1, scrambled_display_word_[i]);
                SDL_Color char_color = (i == selected_char_index_scrambled_) ? color_scrambled_word_cursor_text_ : color_scrambled_word_text_;
                
                int current_char_x = text_render_x_start + i * char_advance_scrambled;
                renderText(char_str, current_char_x, text_render_y, char_color, false, font_scrambled_word_);
            }
        } else if (!scrambled_display_word_.empty()) { // Fallback to default font if font_scrambled_word_ is null
            // Original rendering logic, but with adjusted Y position
            int char_x_start_fallback = 320 - (scrambled_display_word_.length() * (DEFAULT_FONT_SIZE / 2 + 2)) / 2;
            int char_y_fallback = 100; // Use new Y position even for fallback
            for (size_t i = 0; i < scrambled_display_word_.length(); ++i) {
                std::string char_str(1, scrambled_display_word_[i]);
                SDL_Color char_color = (i == selected_char_index_scrambled_) ? color_scrambled_word_cursor_text_ : color_scrambled_word_text_;
                renderText(char_str, char_x_start_fallback + i * (DEFAULT_FONT_SIZE / 2 + 4), char_y_fallback, char_color, false);
            }
        }

        // Display Guessed Word
        if (!current_guess_.empty()) {
            renderText(current_guess_, 320, 200, color_guessed_word_text_, true);
        }

        // Display game messages (Correct, Incorrect)
        if (!game_message_.empty()) {
            Uint32 time_since_message = SDL_GetTicks() - message_display_start_time_;
            // Display "Correct!" permanently in RESULTS_SCREEN, but "Incorrect" temporarily in GAME_SCREEN
            if (current_state_ == GameState::RESULTS_SCREEN || time_since_message < MESSAGE_DISPLAY_DURATION) {
                renderText(game_message_, 320, 230, color_message_text_, true);
            } else {
                game_message_ = ""; // Clear temporary message
            }
        }

        // Display Hint if toggled
        if (m_showHint && !m_currentHintText.empty()) {
            renderText(m_currentHintText, 320, 270, color_message_text_, true, nullptr, 600);
        }

        // Render hints
        renderText("Spacebar to Guess", 320, 380, color_help_text_, true);
        renderText("Backspace to Delete", 320, 400, color_help_text_, true);
        renderText("H for Hint", 320, 420, color_help_text_, true);
        renderText("Arrows: Select | Enter: Use Char", 320, 440, color_help_text_, true, nullptr, 600);
    }

    SDL_RenderPresent(renderer_);
}


void Game::cleanup() {
    if (font_list_name_title_ && font_list_name_title_ != font_) {
        TTF_CloseFont(font_list_name_title_);
        font_list_name_title_ = nullptr;
    }
    if (font_scrambled_word_) {
        TTF_CloseFont(font_scrambled_word_);
        font_scrambled_word_ = nullptr;
    }
    if (font_) {
        TTF_CloseFont(font_);
        font_ = nullptr;
    }
    if (renderer_) {
        SDL_DestroyRenderer(renderer_);
        renderer_ = nullptr;
    }
    if (window_) {
        SDL_DestroyWindow(window_);
        window_ = nullptr;
    }
    TTF_Quit();
    SDL_Quit();
}

