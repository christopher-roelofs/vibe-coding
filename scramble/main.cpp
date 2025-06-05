#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm> // For std::shuffle, std::transform
#include <random>    // For std::mt19937 and std::uniform_int_distribution
#include <fstream>   // For file input
#include <cctype>    // For std::toupper, std::isalpha

// Screen dimensions
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

// Function to render text
SDL_Texture* renderText(const std::string &message, const std::string &fontFile, SDL_Color color, int fontSize, SDL_Renderer *renderer) {
    TTF_Font *font = TTF_OpenFont(fontFile.c_str(), fontSize);
    if (!font) {
        std::cerr << "Failed to load font: " << fontFile << " - TTF_Error: " << TTF_GetError() << std::endl;
        return nullptr;
    }

    SDL_Surface *surf = TTF_RenderText_Blended(font, message.c_str(), color);
    if (!surf) {
        TTF_CloseFont(font);
        std::cerr << "Failed to render text surface - TTF_Error: " << TTF_GetError() << std::endl;
        return nullptr;
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surf);
    if (!texture) {
        std::cerr << "Failed to create texture from surface - SDL_Error: " << SDL_GetError() << std::endl;
    }

    SDL_FreeSurface(surf);
    TTF_CloseFont(font);
    return texture;
}

// Function to pick a random word from a list
std::string pickRandomWord(const std::vector<std::string>& wordList, std::mt19937& rng) {
    if (wordList.empty()) {
        return "";
    }
    std::uniform_int_distribution<int> dist(0, wordList.size() - 1);
    return wordList[dist(rng)];
}

// Function to convert a string to uppercase
std::string toUpper(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c){ return std::toupper(c); });
    return s;
}

// Function to load words from a file
std::vector<std::string> loadWordsFromFile(const std::string& filename) {
    std::vector<std::string> words;
    std::ifstream file(filename);
    std::string line;

    if (file.is_open()) {
        while (std::getline(file, line)) {
            if (!line.empty()) { // Ignore empty lines
                // Basic validation: ensure words are alphabetic, can be extended
                bool isValid = true;
                for (char c : line) {
                    if (!std::isalpha(c)) {
                        isValid = false;
                        break;
                    }
                }
                if (isValid) {
                    words.push_back(toUpper(line));
                }
            }
        }
        file.close();
    } else {
        std::cerr << "Warning: Could not open word file: " << filename << std::endl;
    }
    return words;
}

// Function to scramble a word
std::string scrambleWord(std::string word, std::mt19937& rng) {
    if (word.length() < 2) return word; // No need to shuffle 0 or 1 letter words
    std::string originalWord = word;
    // Ensure it's actually scrambled and not the same as original, unless very short
    if (word.length() > 2) {
        while (word == originalWord) {
            std::shuffle(word.begin(), word.end(), rng);
        }
    }
    return word;
}

enum class GameState {
    PLAYING,
    SHOW_FEEDBACK
};

// Forward declaration for setupNewRound
void setupNewRound(
    const std::vector<std::string>& words, 
    std::mt19937& rng, 
    std::string& currentWord, 
    std::string& scrambledWord, 
    std::string& playerGuess, 
    std::vector<SDL_Texture*>& letterTextures, 
    std::vector<SDL_Rect>& letterRects, 
    int& highlightedIndex, 
    SDL_Renderer* renderer, 
    const std::string& fontPath,
    const SDL_Color& letterColor,
    SDL_Texture*& playerGuessTexture,
    SDL_Rect& playerGuessRect,
    int screenWidth,
    int titleTextureHeight,
    int titleTextureY
);



int main(int argc, char* argv[]) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0) {
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
    SDL_Window* window = SDL_CreateWindow("Word Scramble", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    // Create renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
    
    // Font path - IMPORTANT: User needs to provide this font file or change the path.
    // Common Linux path: "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"
    // Or place e.g. "Arial.ttf" in the same directory as the executable.
    std::string fontPath = "DejaVuSans.ttf"; 
    SDL_Color textColor = {255, 255, 255, 255}; // White
    SDL_Color wordColor = {255, 255, 0, 255}; // Yellow for scrambled word
    SDL_Color guessColor = {173, 216, 230, 255}; // Light Blue for player's guess
    SDL_Color highlightColor = {100, 100, 255, 255}; // Highlight color for selected letter
    SDL_Color feedbackCorrectColor = {0, 200, 0, 255}; // Green for correct
    SDL_Color feedbackIncorrectColor = {200, 0, 0, 255}; // Red for incorrect
    SDL_Color feedbackPromptColor = {200, 200, 200, 255}; // Light gray for prompts

    // Initialize random number generator
    std::random_device rd;
    std::mt19937 rng(rd());

    // Load word list from file
    std::vector<std::string> words = loadWordsFromFile("words.txt");

    // Fallback to default words if file loading failed or file is empty
    if (words.empty()) {
        std::cerr << "Using default word list as words.txt was not found, is empty, or contains no valid words." << std::endl;
        words = {"HELLO", "WORLD", "CASCADE", "MAKER", "PLANT", "TYCOON", "LINUX", "HANDHELD"};
    }
    
    std::string currentWord = pickRandomWord(words, rng);
    std::string scrambledWord = "";
    if (!currentWord.empty()) {
        scrambledWord = scrambleWord(currentWord, rng);
    }

    std::string playerGuess = "";
    // SDL_StartTextInput(); // No longer using direct text input for word formation

    std::vector<SDL_Texture*> scrambledLetterTextures;
    std::vector<SDL_Rect> scrambledLetterRects;
    int highlightedScrambledLetterIndex = 0;

    GameState currentGameState = GameState::PLAYING;
    bool lastGuessWasCorrect = false;
    SDL_Texture* feedbackTextTexture = nullptr;
    SDL_Rect feedbackTextRect = {0,0,0,0};

    SDL_Texture* titleTexture = renderText("Word Scramble Game", fontPath, textColor, 32, renderer);
    SDL_Rect titleRect = {0,0,0,0};
    if (titleTexture) {
        SDL_QueryTexture(titleTexture, NULL, NULL, &titleRect.w, &titleRect.h);
        titleRect.x = (SCREEN_WIDTH - titleRect.w) / 2;
        titleRect.y = 50;
    } else {
        std::cerr << "Failed to render title text. Ensure '" << fontPath << "' is accessible." << std::endl;
    }

    int totalScrambledWordWidth = 0;
    if (!scrambledWord.empty()) {
        int currentX = 0; // Used to calculate total width and position letters
        for (char c : scrambledWord) {
            std::string letterStr(1, c);
            SDL_Texture* letterTex = renderText(letterStr, fontPath, wordColor, 48, renderer);
            if (letterTex) {
                scrambledLetterTextures.push_back(letterTex);
                SDL_Rect letterRect;
                SDL_QueryTexture(letterTex, NULL, NULL, &letterRect.w, &letterRect.h);
                letterRect.y = titleRect.y + (titleTexture ? titleRect.h : 0) + 30;
                // We'll calculate exact X positions after knowing total width
                currentX += letterRect.w + 5; // 5px spacing
                scrambledLetterRects.push_back(letterRect); // Store preliminary rect (y, w, h are correct)
            }
        }
        totalScrambledWordWidth = currentX - 5; // Remove last spacing

        // Now set the X positions for centering
        int startX = (SCREEN_WIDTH - totalScrambledWordWidth) / 2;
        currentX = startX;
        for (size_t i = 0; i < scrambledLetterRects.size(); ++i) {
            scrambledLetterRects[i].x = currentX;
            currentX += scrambledLetterRects[i].w + 5; // 5px spacing
        }
    } else {
        std::cerr << "Scrambled word is empty, cannot display letters." << std::endl;
    }

    SDL_Texture* playerGuessTexture = nullptr;
    SDL_Rect playerGuessRect = {0,0,0,0};
    // Call setupNewRound to initialize the first word
    setupNewRound(words, rng, currentWord, scrambledWord, playerGuess, 
                  scrambledLetterTextures, scrambledLetterRects, highlightedScrambledLetterIndex, 
                  renderer, fontPath, wordColor, playerGuessTexture, playerGuessRect, 
                  SCREEN_WIDTH, (titleTexture ? titleRect.h:0), (titleTexture ? titleRect.y:50));

    // Initial prompt message
    feedbackTextTexture = renderText("Press Space to Submit Guess", fontPath, feedbackPromptColor, 24, renderer);
    if (feedbackTextTexture) {
        SDL_QueryTexture(feedbackTextTexture, NULL, NULL, &feedbackTextRect.w, &feedbackTextRect.h);
        feedbackTextRect.x = (SCREEN_WIDTH - feedbackTextRect.w) / 2;
        int player_guess_render_height = (playerGuessTexture ? playerGuessRect.h : 40); // Approx height for empty guess text area (font size 40)
        int base_y_for_feedback;

        if (!scrambledLetterRects.empty()) { // playerGuessRect.y is defined relative to scrambled letters
            base_y_for_feedback = playerGuessRect.y + player_guess_render_height;
        } else if (titleTexture) { // Fallback based on title and assumed heights
            base_y_for_feedback = (titleRect.y + titleRect.h + 30 + 48) + 20 + player_guess_render_height;
        } else { // Absolute fallback
            base_y_for_feedback = (50 + 30 + 48) + 20 + player_guess_render_height;
        }
        feedbackTextRect.y = base_y_for_feedback + 15; // 15px gap below guess area
    }


    // Game loop
    bool quit = false;
    SDL_Event e;

    while (!quit) {
        // Handle events on queue
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
            if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_ESCAPE) {
                    quit = true;
                }
                if (e.key.keysym.sym == SDLK_LEFT) {
                    if (!scrambledLetterTextures.empty()) {
                        highlightedScrambledLetterIndex = std::max(0, highlightedScrambledLetterIndex - 1);
                    }
                } else if (e.key.keysym.sym == SDLK_RIGHT) {
                    if (!scrambledLetterTextures.empty()) {
                        highlightedScrambledLetterIndex = std::min((int)scrambledLetterTextures.size() - 1, highlightedScrambledLetterIndex + 1);
                    }
                } else if (e.key.keysym.sym == SDLK_RETURN) { // 'Enter' key for selecting letter
                    if (currentGameState == GameState::PLAYING && !scrambledWord.empty() && highlightedScrambledLetterIndex < (int)scrambledWord.length() && playerGuess.length() < currentWord.length()) {
                        playerGuess += scrambledWord[highlightedScrambledLetterIndex];
                        // Re-render playerGuessTexture
                        if (playerGuessTexture) SDL_DestroyTexture(playerGuessTexture);
                        playerGuessTexture = renderText(playerGuess, fontPath, guessColor, 40, renderer);
                        if (playerGuessTexture) {
                            SDL_QueryTexture(playerGuessTexture, NULL, NULL, &playerGuessRect.w, &playerGuessRect.h);
                            playerGuessRect.x = (SCREEN_WIDTH - playerGuessRect.w) / 2;
                            int scrambledLettersY = 0;
                            if (!scrambledLetterRects.empty()) scrambledLettersY = scrambledLetterRects[0].y + scrambledLetterRects[0].h;
                            else if (titleTexture) scrambledLettersY = titleRect.y + titleRect.h + 30 + 48;
                            else scrambledLettersY = 50 + 30 + 48;
                            playerGuessRect.y = scrambledLettersY + 20;
                        } else {
                             std::cerr << "Failed to render player guess text after selection." << std::endl;
                        }
                    }
                } else if (e.key.keysym.sym == SDLK_SPACE) { // Submit guess or continue
                    if (currentGameState == GameState::PLAYING && !playerGuess.empty()) {
                        currentGameState = GameState::SHOW_FEEDBACK;
                        if (feedbackTextTexture) SDL_DestroyTexture(feedbackTextTexture);
                        feedbackTextTexture = nullptr;
                        if (playerGuess == currentWord) {
                            lastGuessWasCorrect = true;
                            feedbackTextTexture = renderText("Correct! Press Space for Next Word", fontPath, feedbackCorrectColor, 24, renderer);
                        } else {
                            lastGuessWasCorrect = false;
                            feedbackTextTexture = renderText("Incorrect! Press Space to Try Again", fontPath, feedbackIncorrectColor, 24, renderer);
                        }
                        if (feedbackTextTexture) {
                            SDL_QueryTexture(feedbackTextTexture, NULL, NULL, &feedbackTextRect.w, &feedbackTextRect.h);
                            feedbackTextRect.x = (SCREEN_WIDTH - feedbackTextRect.w) / 2;
                            int player_guess_render_height = (playerGuessTexture ? playerGuessRect.h : 40); // Approx height for empty guess text area
                            int base_y_for_feedback;

                            if (!scrambledLetterRects.empty()) { // playerGuessRect.y is defined relative to scrambled letters
                                base_y_for_feedback = playerGuessRect.y + player_guess_render_height;
                            } else if (titleTexture) { // Fallback based on title and assumed heights
                                base_y_for_feedback = (titleRect.y + titleRect.h + 30 + 48) + 20 + player_guess_render_height;
                            } else { // Absolute fallback
                                base_y_for_feedback = (50 + 30 + 48) + 20 + player_guess_render_height;
                            }
                            feedbackTextRect.y = base_y_for_feedback + 15; // 15px gap below guess area
                        }
                    } else if (currentGameState == GameState::SHOW_FEEDBACK) {
                        if (lastGuessWasCorrect) {
                            setupNewRound(words, rng, currentWord, scrambledWord, playerGuess, 
                                          scrambledLetterTextures, scrambledLetterRects, highlightedScrambledLetterIndex, 
                                          renderer, fontPath, wordColor, playerGuessTexture, playerGuessRect, 
                                          SCREEN_WIDTH, (titleTexture ? titleRect.h:0), (titleTexture ? titleRect.y:50));
                        } else {
                            // Clear player's guess for retry
                            playerGuess = "";
                            if (playerGuessTexture) SDL_DestroyTexture(playerGuessTexture);
                            playerGuessTexture = nullptr;
                            playerGuessRect.w = 0; playerGuessRect.h = 0;
                        }
                        currentGameState = GameState::PLAYING;
                        if (feedbackTextTexture) SDL_DestroyTexture(feedbackTextTexture);
                        feedbackTextTexture = renderText("Press Space to Submit Guess", fontPath, feedbackPromptColor, 24, renderer); // Reset prompt
                         if (feedbackTextTexture) {
                            SDL_QueryTexture(feedbackTextTexture, NULL, NULL, &feedbackTextRect.w, &feedbackTextRect.h);
                            feedbackTextRect.x = (SCREEN_WIDTH - feedbackTextRect.w) / 2;
                            int player_guess_render_height = (playerGuessTexture ? playerGuessRect.h : 40); // Approx height for empty guess text area
                            int base_y_for_feedback;

                            if (!scrambledLetterRects.empty()) { // playerGuessRect.y is defined relative to scrambled letters
                                base_y_for_feedback = playerGuessRect.y + player_guess_render_height;
                            } else if (titleTexture) { // Fallback based on title and assumed heights
                                base_y_for_feedback = (titleRect.y + titleRect.h + 30 + 48) + 20 + player_guess_render_height;
                            } else { // Absolute fallback
                                base_y_for_feedback = (50 + 30 + 48) + 20 + player_guess_render_height;
                            }
                            feedbackTextRect.y = base_y_for_feedback + 15; // 15px gap below guess area
                        }
                    }
                } else if (e.key.keysym.sym == SDLK_BACKSPACE && !playerGuess.empty()) {
                    playerGuess.pop_back();
                    if (playerGuessTexture) SDL_DestroyTexture(playerGuessTexture);
                    playerGuessTexture = nullptr; 
                    if (!playerGuess.empty()) {
                        playerGuessTexture = renderText(playerGuess, fontPath, guessColor, 40, renderer);
                        if (playerGuessTexture) {
                            SDL_QueryTexture(playerGuessTexture, NULL, NULL, &playerGuessRect.w, &playerGuessRect.h);
                            playerGuessRect.x = (SCREEN_WIDTH - playerGuessRect.w) / 2;
                            int scrambledLettersY = 0;
                            if (!scrambledLetterRects.empty()) scrambledLettersY = scrambledLetterRects[0].y + scrambledLetterRects[0].h;
                            else if (titleTexture) scrambledLettersY = titleRect.y + titleRect.h + 30 + 48;
                            else scrambledLettersY = 50 + 30 + 48;
                            playerGuessRect.y = scrambledLettersY + 20;
                        } else {
                            std::cerr << "Failed to render player guess text after backspace." << std::endl;
                        }
                    } else { // Guess is now empty, ensure texture is null and rect is cleared if needed
                        if (playerGuessTexture) SDL_DestroyTexture(playerGuessTexture);
                        playerGuessTexture = nullptr;
                        playerGuessRect.w = 0; playerGuessRect.h = 0; // Effectively hides it
                    }
                }
            } 
            // Removed SDL_TEXTINPUT handling
            // TODO: Add event handling for SDL_CONTROLLERBUTTONDOWN, SDL_CONTROLLERAXISMOTION for D-pad/face buttons
        }

        // Clear screen (Dark Blue background)
        SDL_SetRenderDrawColor(renderer, 20, 20, 80, 255);
        SDL_RenderClear(renderer);

        // Render title text
        if (titleTexture) {
            SDL_RenderCopy(renderer, titleTexture, NULL, &titleRect);
        }

        // Render individual scrambled letters and highlight
        for (size_t i = 0; i < scrambledLetterTextures.size(); ++i) {
            if (scrambledLetterTextures[i]) {
                SDL_RenderCopy(renderer, scrambledLetterTextures[i], NULL, &scrambledLetterRects[i]);
            }
            if ((int)i == highlightedScrambledLetterIndex) {
                SDL_SetRenderDrawColor(renderer, highlightColor.r, highlightColor.g, highlightColor.b, highlightColor.a);
                SDL_Rect highlightRect = scrambledLetterRects[i];
                highlightRect.x -= 2; highlightRect.y -= 2;
                highlightRect.w += 4; highlightRect.h += 4;
                SDL_RenderDrawRect(renderer, &highlightRect);
            }
        }

        // Render player guess text
        if (playerGuessTexture) {
            SDL_RenderCopy(renderer, playerGuessTexture, NULL, &playerGuessRect);
        }

        // Render feedback text
        if (feedbackTextTexture) {
            SDL_RenderCopy(renderer, feedbackTextTexture, NULL, &feedbackTextRect);
        }

        // Update screen
        SDL_RenderPresent(renderer);
    }

    // Cleanup
    if (titleTexture) {
        SDL_DestroyTexture(titleTexture);
    }
    // if (scrambledWordTexture) SDL_DestroyTexture(scrambledWordTexture); // Removed, letters are individual
    for (SDL_Texture* tex : scrambledLetterTextures) {
        if (tex) SDL_DestroyTexture(tex);
    }
    scrambledLetterTextures.clear();

    if (playerGuessTexture) {
        SDL_DestroyTexture(playerGuessTexture);
    }
    if (feedbackTextTexture) {
        SDL_DestroyTexture(feedbackTextTexture);
    }
    // SDL_StopTextInput(); // Was already removed earlier as we don't call StartTextInput
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();

    return 0;
}

void setupNewRound(
    const std::vector<std::string>& words, 
    std::mt19937& rng, 
    std::string& currentWord_ref, 
    std::string& scrambledWord_ref, 
    std::string& playerGuess_ref, 
    std::vector<SDL_Texture*>& letterTextures_ref, 
    std::vector<SDL_Rect>& letterRects_ref, 
    int& highlightedIndex_ref, 
    SDL_Renderer* renderer, 
    const std::string& fontPath,
    const SDL_Color& letterColor,
    SDL_Texture*& playerGuessTexture_ref,
    SDL_Rect& playerGuessRect_ref,
    int screenWidth,
    int titleTextureHeight_val,
    int titleTextureY_val
) {
    // Clear old letter textures
    for (SDL_Texture* tex : letterTextures_ref) {
        if (tex) SDL_DestroyTexture(tex);
    }
    letterTextures_ref.clear();
    letterRects_ref.clear();

    // Pick and scramble new word
    currentWord_ref = pickRandomWord(words, rng);
    if (!currentWord_ref.empty()) {
        scrambledWord_ref = scrambleWord(currentWord_ref, rng);
    } else {
        scrambledWord_ref = "ERROR"; // Should not happen with a good word list
    }

    // Create textures for new scrambled word
    int totalScrambledWordWidth = 0;
    if (!scrambledWord_ref.empty()) {
        int currentX = 0;
        for (char c : scrambledWord_ref) {
            std::string letterStr(1, c);
            SDL_Texture* letterTex = renderText(letterStr, fontPath, letterColor, 48, renderer);
            if (letterTex) {
                letterTextures_ref.push_back(letterTex);
                SDL_Rect letterRect;
                SDL_QueryTexture(letterTex, NULL, NULL, &letterRect.w, &letterRect.h);
                letterRect.y = titleTextureY_val + titleTextureHeight_val + 30;
                currentX += letterRect.w + 5;
                letterRects_ref.push_back(letterRect);
            }
        }
        totalScrambledWordWidth = currentX - 5;
        int startX = (screenWidth - totalScrambledWordWidth) / 2;
        currentX = startX;
        for (size_t i = 0; i < letterRects_ref.size(); ++i) {
            letterRects_ref[i].x = currentX;
            currentX += letterRects_ref[i].w + 5;
        }
    }

    // Reset player guess
    playerGuess_ref = "";
    if (playerGuessTexture_ref) SDL_DestroyTexture(playerGuessTexture_ref);
    playerGuessTexture_ref = nullptr;
    playerGuessRect_ref.w = 0; playerGuessRect_ref.h = 0;

    // Set the Y position for the player's guess text area
    if (!letterRects_ref.empty() && letterRects_ref[0].h > 0) {
        // Position it 20px below the bottom of the scrambled letters
        playerGuessRect_ref.y = letterRects_ref[0].y + letterRects_ref[0].h + 20;
    } else {
        // Fallback if scrambled letters aren't available (e.g., error in word processing)
        // Estimate position below title and estimated scrambled letter area
        int estimatedScrambledLettersBaseY = titleTextureY_val + titleTextureHeight_val + 30;
        int estimatedScrambledLetterHeight = 48; // Font size for scrambled letters
        playerGuessRect_ref.y = estimatedScrambledLettersBaseY + estimatedScrambledLetterHeight + 20;
    }

    // Reset highlight
    highlightedIndex_ref = 0;
}
