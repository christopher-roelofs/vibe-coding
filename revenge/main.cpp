#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include <vector>
#include <algorithm> // For std::clamp
#include <string>
#include <algorithm> // For std::max and std::min
#include <filesystem> // For listing level files (C++17)
#include <fstream> // For std::ifstream
#include <sstream> // For std::stringstream

#include "Level.h"
#include "Player.h"
#include "Cat.h"
#include "FontManager.h"
#include "TextureManager.h"

// Global debug flag
bool g_debugMode = false;

// Graphics pack settings state for the settings menu
// These are global to be accessible by renderSettingsScreen and input handling in main.
// They are populated/updated when entering the settings screen.
static std::vector<std::string> g_availableGraphicsPacksList_Settings;
static int g_selectedGraphicsPackIndex_Settings = 0;

// Game constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int UI_PANEL_HEIGHT = 80;
const int INITIAL_PLAYER_LIVES = 3;

// Structs for level management
struct IndividualLevelDetail {
    std::string levelTitle;
    int startLineNumberInFile;
};

struct LevelPackInfo {
    std::string filePath;
    std::string packName;
    std::string packDescription;
    std::string packAuthor;
    std::string packDate;
    std::string difficulty;
    std::vector<IndividualLevelDetail> individualLevels;

    std::string getDisplayName() const {
        if (!packName.empty()) {
            return packName;
        }
        std::filesystem::path p(filePath);
        return p.stem().string();
    }
};

// --- SETTINGS PERSISTENCE ---
const std::string SETTINGS_FILE = "settings.txt";

// Saves a setting as a key-value pair.
void saveSettings(const std::string& key, const std::string& value) {
    // For now, we rewrite the whole file. This is simple and fine for a small number of settings.
    // A more complex implementation might read existing values and only change the specified key.
    std::ofstream outFile(SETTINGS_FILE);
    if (outFile.is_open()) {
        outFile << key << "=" << value << std::endl;
        outFile.close();
    }
}

// Loads a setting value for a given key.
// Returns a default value if the key is not found or the file doesn't exist.
std::string loadSettings(const std::string& key, const std::string& defaultValue) {
    std::ifstream inFile(SETTINGS_FILE);
    if (inFile.is_open()) {
        std::string line;
        while (std::getline(inFile, line)) {
            std::istringstream iss(line);
            std::string currentKey;
            if (std::getline(iss, currentKey, '=')) {
                if (currentKey == key) {
                    std::string value;
                    if (std::getline(iss, value)) {
                        // If loading graphics, verify the pack still exists
                        if (key == "graphics") {
                            std::vector<std::string> availablePacks = TextureManager::getAvailableGraphicsPacks();
                            for (const auto& availablePack : availablePacks) {
                                if (availablePack == value) {
                                    return value; // Saved pack is valid
                                }
                            }
                            return defaultValue; // Saved pack is no longer available, return default
                        }
                        return value; // For other potential settings
                    }
                }
            }
        }
    }
    return defaultValue; // Fallback to default
}

// Game state enum
enum class GameState {
    LEVEL_SELECT,
    SETTINGS,
    IN_GAME,
    PLAYER_WINS_LEVEL,
    GAME_OVER
};

// Global variable for the currently selected image pack in the settings menu
// Needs to be accessible by renderSettingsScreen and input handling in main.
// int selectedImagePackIndex = 0; // Pack switching disabled

// Helper function to draw centered text
void drawCenteredText(SDL_Renderer* renderer, const std::string& text, int y, const std::string& fontName, SDL_Color color) {
    int textWidth, textHeight;
    TTF_Font* font = FontManager::getFont(fontName);
    if (font) {
        TTF_SizeText(font, text.c_str(), &textWidth, &textHeight);
        FontManager::drawText(renderer, text, (SCREEN_WIDTH - textWidth) / 2, y, fontName, color);
    }
}

// Helper function to draw wrapped, centered text
int drawWrappedText(SDL_Renderer* renderer, const std::string& text, int y, int maxWidth, const std::string& fontName, SDL_Color color) {
    TTF_Font* font = FontManager::getFont(fontName);
    if (!font) return y;

    std::stringstream ss(text);
    std::string word;
    std::string currentLine;
    int textWidth = 0, textHeight = 0;

    // Get the height of a single line of text to use for line spacing
    TTF_SizeText(font, " ", nullptr, &textHeight);

    while (ss >> word) {
        std::string potentialLine = currentLine.empty() ? word : currentLine + " " + word;
        TTF_SizeText(font, potentialLine.c_str(), &textWidth, nullptr);

        if (textWidth > maxWidth && !currentLine.empty()) {
            drawCenteredText(renderer, currentLine, y, fontName, color);
            y += textHeight; // Move y down for the next line
            currentLine = word; // The new line starts with the word that didn't fit
        } else {
            currentLine = potentialLine;
        }
    }

    // Render the last remaining line
    if (!currentLine.empty()) {
        drawCenteredText(renderer, currentLine, y, fontName, color);
        y += textHeight;
    }

    return y; // Return the Y position for the next element
}

// Rendering functions for different game states
void renderLevelSelectScreen(SDL_Renderer* renderer, const std::vector<LevelPackInfo>& packs, int selectedPackIndex) {
    drawCenteredText(renderer, "Open Revenge", 20, "vcr_osd_36", {255, 255, 255, 255}); // Game Title
    drawCenteredText(renderer, "Select Level Pack", 70, "vcr_osd_24", {255, 255, 255, 255}); // Changed font to vcr_osd_24

    if (packs.empty()) {
        drawCenteredText(renderer, "No level packs found in assets/levels/", SCREEN_HEIGHT / 2, "vcr_osd_24", {255, 100, 100, 255});
    } else {
        // Display the selected pack name prominently
        const LevelPackInfo& selectedPack = packs[selectedPackIndex];
        drawCenteredText(renderer, selectedPack.getDisplayName(), 120, "vcr_osd_24", {255, 255, 0, 255});

        // Display metadata for the selected pack
        int metaY = 180;
        SDL_Color metaColor = {200, 200, 200, 255};
        if (!selectedPack.packDescription.empty()) {
            metaY = drawWrappedText(renderer, selectedPack.packDescription, metaY, SCREEN_WIDTH - 100, "vcr_osd_24", metaColor);
        }
        if (!selectedPack.packAuthor.empty()) {
            std::string authorText = "by " + selectedPack.packAuthor;
            drawCenteredText(renderer, authorText, metaY, "vcr_osd_24", metaColor);
            metaY += 30;
        }
        if (!selectedPack.packDate.empty()) {
            drawCenteredText(renderer, selectedPack.packDate, metaY, "vcr_osd_24", metaColor);
            metaY += 30;
        }

        // Add a small gap
        metaY += 10;

        // Display level count and difficulty
        std::string levelCountText = "Levels: " + std::to_string(selectedPack.individualLevels.size());
        drawCenteredText(renderer, levelCountText, metaY, "vcr_osd_24", metaColor);
        metaY += 30;

        if (!selectedPack.difficulty.empty()) {
            std::string difficultyText = "Difficulty: " + selectedPack.difficulty;
            drawCenteredText(renderer, difficultyText, metaY, "vcr_osd_24", metaColor);
        }
    }

    drawCenteredText(renderer, "Use Left/Right Arrows, Enter to Select", SCREEN_HEIGHT - 80, "vcr_osd_24", {200, 200, 200, 255});
}

void renderGameMessageScreen(SDL_Renderer* renderer, const std::string& message, const std::string& scoreInfo) {
    drawCenteredText(renderer, message, SCREEN_HEIGHT / 3, "vcr_osd_36", {255, 255, 255, 255});
    if (!scoreInfo.empty()) {
        drawCenteredText(renderer, scoreInfo, SCREEN_HEIGHT / 2, "vcr_osd_24", {255, 255, 255, 255});
    }
    drawCenteredText(renderer, "Press Enter to Continue", SCREEN_HEIGHT * 2 / 3, "vcr_osd_24", {200, 200, 200, 255});
}

void renderSettingsScreen(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black background
    SDL_RenderClear(renderer);

    drawCenteredText(renderer, "Settings", 50, "vcr_osd_36", {255, 255, 255, 255});
    drawCenteredText(renderer, "Select Graphics Pack:", 100, "vcr_osd_24", {200, 200, 200, 255});

    int startY = 140;
    int lineHeight = 30;
    std::string currentActivePack = TextureManager::getCurrentGraphicsPackName();

    if (g_availableGraphicsPacksList_Settings.empty()) {
        drawCenteredText(renderer, "No graphics packs found.", startY, "vcr_osd_18", {255, 100, 100, 255});
    } else {
        for (size_t i = 0; i < g_availableGraphicsPacksList_Settings.size(); ++i) {
            std::string packDisplayName = g_availableGraphicsPacksList_Settings[i];
            SDL_Color textColor = {180, 180, 180, 255}; // Default color for non-selected

            if (static_cast<int>(i) == g_selectedGraphicsPackIndex_Settings) {
                packDisplayName = "> " + packDisplayName; // Indicate selection
                textColor = {255, 255, 0, 255}; // Highlight color for selected
            }

            if (g_availableGraphicsPacksList_Settings[i] == currentActivePack) {
                packDisplayName += " (Active)";
                if (static_cast<int>(i) != g_selectedGraphicsPackIndex_Settings) { // Don't overwrite highlight if also selected
                     textColor = {100, 255, 100, 255}; // Green for active but not selected for change
                }
            }
            drawCenteredText(renderer, packDisplayName, startY + (i * lineHeight), "vcr_osd_24", textColor);
        }
    }

    int bottomTextY = SCREEN_HEIGHT - 80;
    drawCenteredText(renderer, "Up/Down to Navigate, Enter to Apply", bottomTextY, "vcr_osd_18", {150, 150, 150, 255});
    drawCenteredText(renderer, "S to return", bottomTextY + 25, "vcr_osd_18", {150, 150, 150, 255});
}

// Level file parsing
std::string parseMetadataValue(const std::string& line, const std::string& key) {
    size_t keyPos = line.find(key);
    if (keyPos == std::string::npos) return "";
    std::string value = line.substr(keyPos + key.length());
    size_t firstChar = value.find_first_not_of(": \t");
    if (firstChar == std::string::npos) return "";
    value = value.substr(firstChar);
    size_t lastChar = value.find_last_not_of(" \t\r\n");
    if (lastChar == std::string::npos) return "";
    return value.substr(0, lastChar + 1);
}

std::vector<LevelPackInfo> discoverLevelPacks(const std::string& directoryPath) {
    std::vector<LevelPackInfo> discoveredPacks;
    try {
        for (const auto& entry : std::filesystem::directory_iterator(directoryPath)) {
            if (entry.is_regular_file() && entry.path().extension() == ".lvl") {
                std::ifstream file(entry.path());
                if (!file.is_open()) continue;

                std::string line;
                int currentLineNumber = 0;
                LevelPackInfo currentPack;
                currentPack.filePath = entry.path().string();
                bool headerProcessed = false;

                while (std::getline(file, line)) {
                    currentLineNumber++;
                    if (line.empty()) continue;
                    
                    if (!headerProcessed && line.rfind("; Name:", 0) == 0) {
                        currentPack.packName = parseMetadataValue(line, "; Name:");
                    } else if (!headerProcessed && line.rfind("; Description:", 0) == 0) {
                        currentPack.packDescription = parseMetadataValue(line, "; Description:");
                    } else if (!headerProcessed && line.rfind("; Author:", 0) == 0) {
                        currentPack.packAuthor = parseMetadataValue(line, "; Author:");
                    } else if (!headerProcessed && line.rfind("; Date:", 0) == 0) {
                        currentPack.packDate = parseMetadataValue(line, "; Date:");
                    } else if (!headerProcessed && line.rfind("; Difficulty:", 0) == 0) {
                        currentPack.difficulty = parseMetadataValue(line, "; Difficulty:");
                    } else if (line.rfind(";", 0) == 0) { 
                        headerProcessed = true; // We are now past the pack header section

                        std::string title = line.substr(1); // Get text after the initial ';'
                        
                        // Trim leading whitespace from the title
                        size_t start = title.find_first_not_of(" \t");
                        if (start == std::string::npos) { // Title was all whitespace or empty after ';'
                            title.clear();
                        } else {
                            // Trim trailing whitespace from the title
                            size_t end = title.find_last_not_of(" \t\r\n");
                            // substr(pos, count). If end is npos (should not happen if start is valid and title had non-space chars),
                            // or if only leading spaces were found, this logic handles it.
                            title = title.substr(start, (end == std::string::npos) ? std::string::npos : (end - start + 1));
                        }
                        
                        // Only add if the title is not empty after trimming
                        if (!title.empty()) {
                            currentPack.individualLevels.push_back({title, currentLineNumber + 1});
                        }
                    } else if (!line.empty()) { // Grid data (doesn't start with ';' and isn't empty)
                        headerProcessed = true; // Mark that we've moved past the header
                    }
                }
                if (!currentPack.individualLevels.empty()) {
                    discoveredPacks.push_back(currentPack);
                }
            }
        }
        std::sort(discoveredPacks.begin(), discoveredPacks.end(), [](const LevelPackInfo& a, const LevelPackInfo& b) {
            return a.getDisplayName() < b.getDisplayName();
        });
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error accessing level directory: " << e.what() << std::endl;
    }
    return discoveredPacks;
}

// Function to update camera position to follow the player
// Uses global constants SCREEN_WIDTH, SCREEN_HEIGHT, UI_PANEL_HEIGHT
void updateCamera(Level& currentLevel, int playerGridX, int playerGridY, int& camX, int& camY) {
    int TILE_SIZE = currentLevel.getTileSize();
    if (TILE_SIZE <= 0) TILE_SIZE = 32;

    int playerWorldX = playerGridX * TILE_SIZE;
    int playerWorldY = playerGridY * TILE_SIZE;
    int levelPixelWidth = currentLevel.getWidth() * TILE_SIZE;
    int levelPixelHeight = currentLevel.getHeight() * TILE_SIZE;

    // Calculate the ideal offset to center the player's TILE.
    int idealOffsetX = (SCREEN_WIDTH / 2) - (playerWorldX + TILE_SIZE / 2);
    int idealOffsetY = ((SCREEN_HEIGHT - UI_PANEL_HEIGHT) / 2) - (playerWorldY + TILE_SIZE / 2);

    // Clamp the offset so the camera doesn't show areas outside the level.
    int gameViewHeight = SCREEN_HEIGHT - UI_PANEL_HEIGHT;

    // For levels smaller than the screen, center them.
    if (levelPixelWidth <= SCREEN_WIDTH) {
        camX = (SCREEN_WIDTH - levelPixelWidth) / 2;
    } else {
        // For larger levels, clamp the scrolling offset.
        camX = std::clamp(idealOffsetX, SCREEN_WIDTH - levelPixelWidth, 0);
    }

    if (levelPixelHeight <= gameViewHeight) {
        camY = (gameViewHeight - levelPixelHeight) / 2;
    } else {
        camY = std::clamp(idealOffsetY, gameViewHeight - levelPixelHeight, 0);
    }
}

// Main game function
int main(int argc, char* argv[]) {
    // Parse command-line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--debug") {
            g_debugMode = true;
        }
    }

    if (g_debugMode) {
        // << "Debug mode enabled." << std::endl;
    }

    // --- INITIALIZATION ---
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }
    if (IMG_Init(IMG_INIT_PNG) == 0) {
        std::cerr << "IMG_Init Error: " << IMG_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }
    if (TTF_Init() == -1) {
        std::cerr << "TTF_Init Error: " << TTF_GetError() << std::endl;
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Open Revenge", 100, 100, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == nullptr) {
        SDL_DestroyWindow(window);
        std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    // --- ASSET LOADING ---
    FontManager::init();
    FontManager::loadFont("vcr_osd_36", "assets/fonts/VCR_OSD_MONO.ttf", 36);
    FontManager::loadFont("vcr_osd_24", "assets/fonts/VCR_OSD_MONO.ttf", 24);
    FontManager::loadFont("vcr_osd_18", "assets/fonts/VCR_OSD_MONO.ttf", 18);

    // Register all game textures with TextureManager
    TextureManager::registerTexture("wall", "wall.png");
    TextureManager::registerTexture("cheese", "cheese.png");
    TextureManager::registerTexture("cat", "cat.png");
    TextureManager::registerTexture("mouse", "mouse.png"); // Player
    TextureManager::registerTexture("block", "block.png");
    TextureManager::registerTexture("empty", "void.png");  // Background tile for empty spaces
    TextureManager::registerTexture("mousetrap", "mousetrap.png");
    TextureManager::registerTexture("hole", "hole.png");
    // Additional potentially used textures:
    TextureManager::registerTexture("lives", "lives.png"); // For UI, if used
    TextureManager::registerTexture("cat_awaiting", "cat_awaiting.png"); // Alternative cat state, if used

    // Load textures from the last used or default graphics pack
    std::string initialPack = loadSettings("graphics", "default");
    if (!TextureManager::setGraphicsPack(initialPack, renderer)) {
        std::cerr << "FATAL: Failed to load initial graphics pack '" << initialPack << "'. Check assets/images/ and texture filenames. Exiting." << std::endl;
        // Cleanup and exit
        FontManager::quit();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        return 1; // Indicate failure
    }
    //std::cout << "Main: Initial graphics pack 'original' loaded successfully." << std::endl;

    // --- GAME VARIABLES ---
    GameState currentState = GameState::LEVEL_SELECT;
    static GameState g_previousGameState = GameState::LEVEL_SELECT; // Store state before entering settings
    bool quit = false;
    SDL_Event e;

    Level level;
    // Player stats that persist across levels in a pack
    int playerScore = 0;
    int playerLives = INITIAL_PLAYER_LIVES;

    std::vector<LevelPackInfo> levelPacks = discoverLevelPacks("assets/levels");
    int currentSelectedLevelPackIndex = 0;
    int currentLevelIndexInPack = 0;
    LevelPackInfo currentSelectedLevelPackInfo;

    int cameraX = 0;
    int cameraY = 0;

    // --- GAME LOOP ---
    while (!quit) {
        // --- EVENT HANDLING ---
        while (SDL_PollEvent(&e) != 0) {
        // Log current state at the beginning of every event poll
        //std::cout << "[EVENT_LOOP] GameState: " << static_cast<int>(currentState) << " | EventType: " << e.type << std::endl;
            if (e.type == SDL_QUIT) {
                quit = true;
            }

            // Universal 'S' key for settings
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_s) {
                if (currentState != GameState::SETTINGS) { // Avoid re-entering settings from settings
                    g_previousGameState = currentState;
                    currentState = GameState::SETTINGS;
                    // Populate settings screen data
                    g_availableGraphicsPacksList_Settings = TextureManager::getAvailableGraphicsPacks();
                    if (!g_availableGraphicsPacksList_Settings.empty()) {
                        std::string currentActivePack = TextureManager::getCurrentGraphicsPackName();
                        g_selectedGraphicsPackIndex_Settings = 0; // Default to first pack
                        for (size_t i = 0; i < g_availableGraphicsPacksList_Settings.size(); ++i) {
                            if (g_availableGraphicsPacksList_Settings[i] == currentActivePack) {
                                g_selectedGraphicsPackIndex_Settings = static_cast<int>(i);
                                break;
                            }
                        }
                    } else {
                         g_selectedGraphicsPackIndex_Settings = 0; // No packs, so index is 0 but list is empty
                    }
                    //std::cout << "[STATE_CHANGE] Universal S key: Switched to GameState::SETTINGS from " << static_cast<int>(g_previousGameState) << std::endl;
                    continue; // Consume the 'S' event here, skip other state-specific handling for this event
                }
            }

            // TITLE_SCREEN state and its event handling removed.
            // The game now starts directly in LEVEL_SELECT.

            if (currentState == GameState::SETTINGS) {
                //std::cout << "[MAIN.CPP] Processing event in GameState::SETTINGS. Event type: " << e.type << std::endl;
                if (e.type == SDL_KEYDOWN) {
                    // Note: The global 'S' key handler (lines ~440-458) has 'continue' so it won't fall through to here when entering settings.
                    // This 'S' key check is specifically for *exiting* settings.
                    if (e.key.keysym.sym == SDLK_s) { 
                        currentState = g_previousGameState; // Return to where we were before settings
                        //std::cout << "[STATE_CHANGE] S from Settings: Switched to GameState: " << static_cast<int>(g_previousGameState) << std::endl;
                    } else if (e.key.keysym.sym == SDLK_UP) {
                        if (!g_availableGraphicsPacksList_Settings.empty()) {
                            g_selectedGraphicsPackIndex_Settings = (g_selectedGraphicsPackIndex_Settings - 1 + g_availableGraphicsPacksList_Settings.size()) % g_availableGraphicsPacksList_Settings.size();
                        }
                    } else if (e.key.keysym.sym == SDLK_DOWN) {
                        if (!g_availableGraphicsPacksList_Settings.empty()) {
                            g_selectedGraphicsPackIndex_Settings = (g_selectedGraphicsPackIndex_Settings + 1) % g_availableGraphicsPacksList_Settings.size();
                        }
                    } else if (e.key.keysym.sym == SDLK_RETURN) {
                        if (!g_availableGraphicsPacksList_Settings.empty() && 
                            g_selectedGraphicsPackIndex_Settings >= 0 && 
                            g_selectedGraphicsPackIndex_Settings < static_cast<int>(g_availableGraphicsPacksList_Settings.size())) {
                            const std::string& chosenPackName = g_availableGraphicsPacksList_Settings[g_selectedGraphicsPackIndex_Settings];
                            //std::cout << "Settings: User selected pack: " << chosenPackName << std::endl;
                            if (!TextureManager::setGraphicsPack(chosenPackName, renderer)) {
                                std::cerr << "Settings: Failed to apply graphics pack: " << chosenPackName << std::endl;
                                // Optionally, set a status message to display on screen
                            } else {
                                saveSettings("graphics", chosenPackName); // Persist the new choice
                                //std::cout << "Settings: Successfully applied and saved graphics pack: " << chosenPackName << std::endl;
                            }
                        }
                    }
                }
            } else if (currentState == GameState::LEVEL_SELECT) {
                if (e.type == SDL_KEYDOWN) {
                    switch (e.key.keysym.sym) {
                        case SDLK_LEFT:
                            if (!levelPacks.empty()) {
                                currentSelectedLevelPackIndex = (currentSelectedLevelPackIndex - 1 + levelPacks.size()) % levelPacks.size();
                            }
                            break;
                        case SDLK_RIGHT:
                            if (!levelPacks.empty()) {
                                currentSelectedLevelPackIndex = (currentSelectedLevelPackIndex + 1) % levelPacks.size();
                            }
                            break;
                        case SDLK_ESCAPE: // Add Escape key handling
                            quit = true;
                            break;
                        case SDLK_RETURN:
                            if (!levelPacks.empty()) {
                                currentSelectedLevelPackInfo = levelPacks[currentSelectedLevelPackIndex];
                                currentLevelIndexInPack = 0;
                                // Reset stats for new pack
                                playerScore = 0;
                                playerLives = INITIAL_PLAYER_LIVES;
                                // Load the first level
                                level.load(currentSelectedLevelPackInfo.filePath, currentSelectedLevelPackInfo.individualLevels[currentLevelIndexInPack].startLineNumberInFile);
                                Player* initialPlayer = level.getPlayer();
                                if(initialPlayer) {
                                    initialPlayer->setScore(playerScore);
                                    initialPlayer->setLives(playerLives);
                                    updateCamera(level, initialPlayer->getX(), initialPlayer->getY(), cameraX, cameraY); // Ensure camera is set on initial load
                                }
                                currentState = GameState::IN_GAME;
                            }
                            break;
                    }
                }
            } else if (currentState == GameState::IN_GAME) {
                Player* currentPlayer = level.getPlayer();
                if (e.type == SDL_KEYDOWN && currentPlayer) {
                    bool playerMoved = false;
                    int dx = 0, dy = 0;
                    switch (e.key.keysym.sym) {
                        case SDLK_ESCAPE:
                            currentState = GameState::LEVEL_SELECT;
                            break;
                        case SDLK_n: // Skip to next level
                            currentLevelIndexInPack++;
                            if (currentLevelIndexInPack < currentSelectedLevelPackInfo.individualLevels.size()) {
                                // Preserve score and lives when skipping
                                playerScore = currentPlayer->getScore();
                                playerLives = currentPlayer->getLives();

                                level.load(currentSelectedLevelPackInfo.filePath, currentSelectedLevelPackInfo.individualLevels[currentLevelIndexInPack].startLineNumberInFile);
                                Player* nextLevelPlayer = level.getPlayer();
                                if(nextLevelPlayer) { 
                                    nextLevelPlayer->setScore(playerScore);
                                    nextLevelPlayer->setLives(playerLives);
                                    updateCamera(level, nextLevelPlayer->getX(), nextLevelPlayer->getY(), cameraX, cameraY);
                                }
                            } else {
                                // Finished the last level
                                currentState = GameState::PLAYER_WINS_LEVEL;
                            }
                            break;
                        case SDLK_p: // Go to previous level
                            if (currentLevelIndexInPack > 0) {
                                currentLevelIndexInPack--;
                                // Preserve score and lives
                                playerScore = currentPlayer->getScore();
                                playerLives = currentPlayer->getLives();

                                level.load(currentSelectedLevelPackInfo.filePath, currentSelectedLevelPackInfo.individualLevels[currentLevelIndexInPack].startLineNumberInFile);
                                Player* prevLevelPlayer = level.getPlayer();
                                if(prevLevelPlayer) {
                                    prevLevelPlayer->setScore(playerScore);
                                    prevLevelPlayer->setLives(playerLives);
                                    updateCamera(level, prevLevelPlayer->getX(), prevLevelPlayer->getY(), cameraX, cameraY);
                                }
                            }
                            break;
                        case SDLK_UP:    dy = -1; playerMoved = true; break;
                        case SDLK_DOWN:  dy =  1; playerMoved = true; break;
                        case SDLK_LEFT:  dx = -1; playerMoved = true; break;
                        case SDLK_RIGHT: dx =  1; playerMoved = true; break;
                        case SDLK_r: // Restart level
                            // First, capture the most up-to-date stats from the current player object.
                            playerScore = currentPlayer->getScore();
                            playerLives = currentPlayer->getLives();

                            level.load(currentSelectedLevelPackInfo.filePath, currentSelectedLevelPackInfo.individualLevels[currentLevelIndexInPack].startLineNumberInFile);
                            Player* restartPlayer = level.getPlayer();
                            if(restartPlayer) {
                                restartPlayer->setScore(playerScore); // Restore score
                                restartPlayer->setLives(playerLives); // Restore lives
                                updateCamera(level, restartPlayer->getX(), restartPlayer->getY(), cameraX, cameraY); // Recenter camera
                            }
                            break;
                    }
                    if (playerMoved) {
                        MoveResult result = currentPlayer->move(dx, dy, level);
                        switch (result) {
                            case MoveResult::SUCCESS:
                                level.updateTrappedCats();
                                break;
                            case MoveResult::BLOCKED_CAT:
                                if (currentPlayer->getLives() <= 0) {
                                    currentState = GameState::GAME_OVER;
                                } else {
                                    level.resetAllPositions();
                                    updateCamera(level, currentPlayer->getX(), currentPlayer->getY(), cameraX, cameraY);
                                }
                                break;
                            case MoveResult::BLOCKED_TRAP:
                                // Remove the trap at the location the player tried to move to
                                level.removeGameObjectAt(currentPlayer->getX() + dx, currentPlayer->getY() + dy);
                                if (currentPlayer->getLives() <= 0) {
                                    currentState = GameState::GAME_OVER;
                                } else {
                                    // Reset positions of remaining objects (player, cats)
                                    level.resetAllPositions();
                                    updateCamera(level, currentPlayer->getX(), currentPlayer->getY(), cameraX, cameraY);
                                }
                                break;
                            case MoveResult::BLOCKED_WALL:
                            case MoveResult::BLOCKED_CHAIN:
                                // No action needed, move was just blocked
                                break;
                            case MoveResult::SUCCESS_HOLE:
                                // Player is stuck, no special action needed here
                                break;
                        }
                    }
                }
            } else if (currentState == GameState::PLAYER_WINS_LEVEL || currentState == GameState::GAME_OVER) {
                if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_RETURN) {
                    currentState = GameState::LEVEL_SELECT;
                }
            }
        }

        // --- GAME LOGIC / STATE UPDATES ---
        if (currentState == GameState::IN_GAME) {
            Player* currentPlayer = level.getPlayer();
            if (!currentPlayer) { // If player doesn't exist (e.g., bad level file), go back to select screen
                currentState = GameState::LEVEL_SELECT;
                continue;
            }
            
            // Update player stats from the level's player object
            playerScore = currentPlayer->getScore();
            playerLives = currentPlayer->getLives();

            // Update all cats
            for (const auto& obj : level.getGameObjects()) {
                if (auto* cat = dynamic_cast<Cat*>(obj.get())) {
                    cat->update(level);
                }
            }

            // Check if cats trapped themselves or were trapped by player
            level.updateTrappedCats();

            // Check for win condition
            if (level.getCheeseCount() == 0 && level.getCatCount() == 0) {
                currentLevelIndexInPack++;
                if (currentLevelIndexInPack < currentSelectedLevelPackInfo.individualLevels.size()) {
                    // Load next level in the pack
                    level.load(currentSelectedLevelPackInfo.filePath, currentSelectedLevelPackInfo.individualLevels[currentLevelIndexInPack].startLineNumberInFile);
                     Player* nextLevelPlayer = level.getPlayer();
                     if(nextLevelPlayer) { // Sync stats on new player object
                       nextLevelPlayer->setScore(playerScore);
                       nextLevelPlayer->setLives(playerLives);
                       updateCamera(level, nextLevelPlayer->getX(), nextLevelPlayer->getY(), cameraX, cameraY); // Ensure camera is set for next level
                    }
                } else {
                    currentState = GameState::PLAYER_WINS_LEVEL; // Won the whole pack
                }
            }
            
            // Update camera
            Player* currentPlayerForCamera = level.getPlayer(); // Get current player state
            if (currentPlayerForCamera) {
                updateCamera(level, currentPlayerForCamera->getX(), currentPlayerForCamera->getY(), cameraX, cameraY);
            } else {
                // Optional: Handle camera if player is somehow null (e.g., reset to 0,0)
                cameraX = 0;
                cameraY = 0;
            }
        }

        // --- RENDERING ---
        // Set background color based on state and clear the screen
        if (currentState == GameState::LEVEL_SELECT) {
            SDL_SetRenderDrawColor(renderer, 10, 20, 30, 255);
        } else if (currentState == GameState::IN_GAME) {
            SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
        } else { // Default to black
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        }
        SDL_RenderClear(renderer);

        // Draw content for the current state
        if (currentState == GameState::LEVEL_SELECT) {
            renderLevelSelectScreen(renderer, levelPacks, currentSelectedLevelPackIndex);
        } else if (currentState == GameState::SETTINGS) {
            renderSettingsScreen(renderer); // Ensure this is called
        } else if (currentState == GameState::IN_GAME) {
            // Render game world with camera offset
            level.render(renderer, cameraX, cameraY + UI_PANEL_HEIGHT);

            // Render UI Panel (fixed position)
            SDL_Rect uiPanelRect = {0, 0, SCREEN_WIDTH, UI_PANEL_HEIGHT};
            SDL_SetRenderDrawColor(renderer, 30, 30, 40, 220);
            SDL_RenderFillRect(renderer, &uiPanelRect);

            std::stringstream scoreText, livesText, levelText, packText;
            packText << "Pack: " << currentSelectedLevelPackInfo.getDisplayName();
            levelText << "Level " << (currentLevelIndexInPack + 1) << "/" << currentSelectedLevelPackInfo.individualLevels.size() << ": " << currentSelectedLevelPackInfo.individualLevels[currentLevelIndexInPack].levelTitle;
            scoreText << "Score: " << playerScore;
            livesText << "Lives: " << playerLives;

            FontManager::drawText(renderer, packText.str(), 10, 10, "vcr_osd_18", {255, 255, 255, 255});
            FontManager::drawText(renderer, levelText.str(), 10, 30, "vcr_osd_18", {255, 255, 255, 255});
            FontManager::drawText(renderer, scoreText.str(), SCREEN_WIDTH - 150, 10, "vcr_osd_18", {255, 255, 255, 255});
            FontManager::drawText(renderer, livesText.str(), SCREEN_WIDTH - 150, 30, "vcr_osd_18", {255, 255, 255, 255});
        } else if (currentState == GameState::PLAYER_WINS_LEVEL) {
            renderGameMessageScreen(renderer, "Level Pack Complete!", "Final Score: " + std::to_string(playerScore));
        } else if (currentState == GameState::GAME_OVER) {
            renderGameMessageScreen(renderer, "Game Over", "Final Score: " + std::to_string(playerScore));
        }

        // Present the final rendered frame to the screen
        SDL_RenderPresent(renderer);
    }

    // --- CLEANUP ---
    TextureManager::clear();
    FontManager::quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    return 0;
}
