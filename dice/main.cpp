#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <cmath>
#include <string>
#include <map>
#include <fstream>

const int LOGICAL_WIDTH = 640;
const int LOGICAL_HEIGHT = 480;
const int WINDOW_WIDTH = 1280;  // 2x scale by default
const int WINDOW_HEIGHT = 960;
const int GRID_WIDTH = 6;
const int GRID_HEIGHT = 4;
const int TILE_SIZE = 90;
const int TILE_MARGIN = 10;
const int GRID_OFFSET_X = (LOGICAL_WIDTH - (GRID_WIDTH * (TILE_SIZE + TILE_MARGIN) - TILE_MARGIN)) / 2;
const int GRID_OFFSET_Y = 80;

struct Animation {
    float scale;
    float glowIntensity;
    int mergeCount;
    float rotation;      // Current rotation angle in degrees
    float rotationSpeed; // Rotation speed in degrees per second
    float fallStartY;    // Y position when fall started
    float currentY;      // Current Y position during fall
    Uint32 startTime;
    bool active;
    bool falling;        // Is this dice currently falling?
    
    Animation() : scale(1.0f), glowIntensity(0.0f), mergeCount(0), 
                  rotation(0.0f), rotationSpeed(0.0f), fallStartY(0.0f), 
                  currentY(0.0f), startTime(0), active(false), falling(false) {}
};

struct FloatingText {
    std::string text;
    int x, y;
    float alpha;
    Uint32 startTime;
    bool active;
    
    FloatingText() : text(""), x(0), y(0), alpha(255.0f), startTime(0), active(false) {}
};

enum SpecialDiceType {
    NORMAL = -1,
    HEART = 0,      // 0.png
    SKULL = 1,      // 1.png  
    FIST = 2,       // 2.png
    SWORD = 3,      // 3.png
    SHIELD = 4,     // 4.png
    SCROLL = 5,     // 5.png
    PLUS = 6,       // 6.png
    MINUS = 7,      // 7.png
    DIVIDE = 8,     // 8.png
    MULTIPLY = 9,   // 9.png
    QUESTION = 10,  // 10.png
    DOLLAR = 11     // 11.png
};

class Tile {
public:
    int value;
    bool selected;
    int mergeCount;  // How many times this tile has been created through merging
    SpecialDiceType specialType;
    Animation anim;
    
    Tile() : value(0), selected(false), mergeCount(0), specialType(NORMAL) {}
    Tile(int val) : value(val), selected(false), mergeCount(0), specialType(NORMAL) {}
};

class Game {
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    TTF_Font* font;
    TTF_Font* smallFont;
    std::map<std::string, SDL_Texture*> diceTextures;
    std::map<int, SDL_Texture*> specialDiceTextures;
    std::vector<std::string> colorNames = {"red", "blue", "green", "yellow", "purple", "black"};
    std::vector<std::vector<Tile>> grid;
    std::vector<FloatingText> floatingTexts;
    int cursorX, cursorY;
    int lastSelectedX, lastSelectedY; // Position of last dice selected with Enter
    float cursorDisplayX, cursorDisplayY; // Smooth animated cursor position
    float cursorTargetX, cursorTargetY;   // Target position for cursor animation
    int score;
    int highScore;
    std::mt19937 rng;
    std::discrete_distribution<> spawnDist;
    
public:
    Game() : window(nullptr), renderer(nullptr), font(nullptr), smallFont(nullptr),
             cursorX(0), cursorY(0), lastSelectedX(-1), lastSelectedY(-1), 
             cursorDisplayX(0.0f), cursorDisplayY(0.0f), cursorTargetX(0.0f), cursorTargetY(0.0f),
             score(0), highScore(0), rng(std::random_device{}()), spawnDist({50, 35, 15}) {
        grid.resize(GRID_HEIGHT, std::vector<Tile>(GRID_WIDTH));
        floatingTexts.resize(10); // Pool of floating texts
    }
    
    ~Game() {
        cleanup();
    }
    
    bool init() {
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
            return false;
        }
        
        if (TTF_Init() == -1) {
            std::cerr << "SDL_ttf could not initialize! SDL_ttf Error: " << TTF_GetError() << std::endl;
            return false;
        }
        
        int imgFlags = IMG_INIT_PNG;
        if (!(IMG_Init(imgFlags) & imgFlags)) {
            std::cerr << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << std::endl;
            return false;
        }
        
        window = SDL_CreateWindow("Dice Merge", 
                                  SDL_WINDOWPOS_CENTERED, 
                                  SDL_WINDOWPOS_CENTERED, 
                                  WINDOW_WIDTH, 
                                  WINDOW_HEIGHT, 
                                  SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
        if (!window) {
            std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
            return false;
        }
        
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        if (!renderer) {
            std::cerr << "Renderer could not be created! SDL Error: " << SDL_GetError() << std::endl;
            return false;
        }
        
        // Set logical rendering size for scaling
        if (SDL_RenderSetLogicalSize(renderer, LOGICAL_WIDTH, LOGICAL_HEIGHT) < 0) {
            std::cerr << "Failed to set logical size! SDL Error: " << SDL_GetError() << std::endl;
            return false;
        }
        
        font = TTF_OpenFont("assets/fonts/InterVariable.ttf", 32);
        if (!font) {
            std::cerr << "Failed to load font! SDL_ttf Error: " << TTF_GetError() << std::endl;
            return false;
        }
        
        smallFont = TTF_OpenFont("assets/fonts/InterVariable.ttf", 18);
        if (!smallFont) {
            std::cerr << "Failed to load small font! SDL_ttf Error: " << TTF_GetError() << std::endl;
            return false;
        }
        
        if (!loadDiceTextures()) {
            std::cerr << "Failed to load dice textures!" << std::endl;
            return false;
        }
        
        if (!loadSpecialDiceSprites()) {
            std::cerr << "Failed to load special dice sprites!" << std::endl;
            return false;
        }
        
        loadHighScore();
        initializeGrid();
        updateCursorTarget(); // Initialize cursor target position
        
        return true;
    }
    
    bool loadDiceTextures() {
        for (const std::string& color : colorNames) {
            for (int value = 1; value <= 6; value++) {
                std::string filename = "assets/images/128px/" + color + std::to_string(value) + "-128.png";
                std::string key = color + std::to_string(value);
                
                SDL_Surface* surface = IMG_Load(filename.c_str());
                if (!surface) {
                    std::cerr << "Failed to load " << filename << "! SDL_image Error: " << IMG_GetError() << std::endl;
                    return false;
                }
                
                SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
                SDL_FreeSurface(surface);
                
                if (!texture) {
                    std::cerr << "Failed to create texture from " << filename << "! SDL Error: " << SDL_GetError() << std::endl;
                    return false;
                }
                
                diceTextures[key] = texture;
            }
        }
        return true;
    }
    
    bool loadSpecialDiceSprites() {
        // Load individual special dice PNG files from cute folder
        std::vector<int> specialTypes = {HEART, SKULL, FIST, SWORD, SHIELD, SCROLL, 
                                        PLUS, MINUS, DIVIDE, MULTIPLY, QUESTION, DOLLAR};
        
        for (int specialType : specialTypes) {
            std::string filename = "assets/images/cute/" + std::to_string(specialType) + ".png";
            
            SDL_Surface* surface = IMG_Load(filename.c_str());
            if (!surface) {
                std::cerr << "Failed to load " << filename << "! SDL_image Error: " << IMG_GetError() << std::endl;
                return false;
            }
            
            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_FreeSurface(surface);
            
            if (!texture) {
                std::cerr << "Failed to create texture from " << filename << "! SDL Error: " << SDL_GetError() << std::endl;
                return false;
            }
            
            specialDiceTextures[specialType] = texture;
        }
        
        return true;
    }
    
    SDL_Texture* getDiceTexture(int value) {
        if (value < 1 || value > 6) return nullptr;
        
        // Map dice values to colors: 1=red, 2=blue, 3=green, 4=yellow, 5=purple, 6=black
        std::string color = colorNames[value - 1];
        std::string key = color + std::to_string(value);
        
        auto it = diceTextures.find(key);
        return (it != diceTextures.end()) ? it->second : nullptr;
    }
    
    void loadHighScore() {
        std::ifstream file("highscore");
        if (file.is_open()) {
            file >> highScore;
            file.close();
        } else {
            highScore = 0;
        }
    }
    
    void saveHighScore() {
        std::ofstream file("highscore");
        if (file.is_open()) {
            file << highScore;
            file.close();
        }
    }
    
    void initializeGrid() {
        for (int y = 0; y < GRID_HEIGHT; y++) {
            for (int x = 0; x < GRID_WIDTH; x++) {
                int value = spawnNewValue();
                if (value == -1) {
                    grid[y][x].value = 0; // Special dice have value 0 for game logic
                    grid[y][x].specialType = getRandomSpecialType();
                } else {
                    grid[y][x].value = value;
                    grid[y][x].specialType = NORMAL;
                }
            }
        }
    }
    
    int spawnNewValue() {
        // 5% chance for special dice
        if (rng() % 100 < 5) {
            return -1; // Special dice marker
        }
        return spawnDist(rng) + 1; // 1, 2, or 3
    }
    
    SpecialDiceType getRandomSpecialType() {
        // Weighted spawn chances for different types of special dice
        std::vector<SpecialDiceType> types;
        
        // Beneficial dice (higher spawn rate) - 3x weight each
        for (int i = 0; i < 3; i++) {
            types.push_back(HEART);     // Remove lowest
            types.push_back(PLUS);      // +50 points
            types.push_back(MULTIPLY);  // x2 score
            types.push_back(DOLLAR);    // +100 points
            types.push_back(SHIELD);    // Protection
        }
        
        // Neutral/utility dice (medium spawn rate) - 2x weight each
        for (int i = 0; i < 2; i++) {
            types.push_back(FIST);      // Smash adjacent
            types.push_back(SCROLL);    // Copy dice
            types.push_back(MINUS);     // Reduce values
            types.push_back(DIVIDE);    // Split dice
            types.push_back(QUESTION);  // Random effect
        }
        
        // Risky/destructive dice (low spawn rate) - 1x weight each
        types.push_back(SKULL);         // Remove highest
        types.push_back(SWORD);         // Attack with points
        
        return types[rng() % types.size()];
    }
    
    void run() {
        bool running = true;
        SDL_Event event;
        
        while (running) {
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) {
                    running = false;
                } else if (event.type == SDL_KEYDOWN) {
                    handleKeyPress(event.key.keysym.sym);
                }
            }
            
            updateAnimations();
            render();
            SDL_Delay(16); // ~60 FPS
        }
    }
    
    void handleKeyPress(SDL_Keycode key) {
        switch (key) {
            case SDLK_UP:
                if (cursorY > 0) {
                    cursorY--;
                    updateCursorTarget();
                }
                break;
            case SDLK_DOWN:
                if (cursorY < GRID_HEIGHT - 1) {
                    cursorY++;
                    updateCursorTarget();
                }
                break;
            case SDLK_LEFT:
                if (cursorX > 0) {
                    cursorX--;
                    updateCursorTarget();
                }
                break;
            case SDLK_RIGHT:
                if (cursorX < GRID_WIDTH - 1) {
                    cursorX++;
                    updateCursorTarget();
                }
                break;
            case SDLK_RETURN:
                selectTile(cursorX, cursorY);
                break;
            case SDLK_ESCAPE:
                clearSelection();
                break;
        }
    }
    
    void updateCursorTarget() {
        cursorTargetX = cursorX * (TILE_SIZE + TILE_MARGIN);
        cursorTargetY = cursorY * (TILE_SIZE + TILE_MARGIN);
    }
    
    void selectTile(int x, int y) {
        if (grid[y][x].value == 0 && grid[y][x].specialType == NORMAL) return;
        
        // Toggle selection
        grid[y][x].selected = !grid[y][x].selected;
        
        // Track the last position where we pressed Enter (for merge target)
        // Only update if we're selecting (not deselecting)
        if (grid[y][x].selected) {
            lastSelectedX = x;
            lastSelectedY = y;
        }
        
        // Count total selected tiles
        int totalSelected = 0;
        int selectedValue = -1;
        for (int sy = 0; sy < GRID_HEIGHT; sy++) {
            for (int sx = 0; sx < GRID_WIDTH; sx++) {
                if (grid[sy][sx].selected) {
                    totalSelected++;
                    if (selectedValue == -1) {
                        selectedValue = grid[sy][sx].value;
                    }
                }
            }
        }
        
        // If we have 3+ selected tiles
        if (totalSelected >= 3) {
            // Check if they're all connected (find any selected tile that can reach all others)
            bool isConnected = false;
            int referenceX = -1, referenceY = -1;
            
            // Try each selected tile as a reference point for connectivity
            for (int sy = 0; sy < GRID_HEIGHT && !isConnected; sy++) {
                for (int sx = 0; sx < GRID_WIDTH && !isConnected; sx++) {
                    if (grid[sy][sx].selected) {
                        int connectedCount = countSelectedAdjacent(sx, sy);
                        if (connectedCount >= 3 && connectedCount == totalSelected) {
                            isConnected = true;
                            referenceX = sx;
                            referenceY = sy;
                        }
                    }
                }
            }
            
            if (isConnected) {
                // All selected tiles are connected - perform merge at current cursor position
                performMerge(x, y);
            } else {
                // Not all connected or different values - deselect all
                clearSelection();
            }
        }
    }
    
    int countSelectedAdjacent(int x, int y) {
        int value = grid[y][x].value;
        
        // If starting tile is special, find a regular dice value in the selection
        if (grid[y][x].specialType != NORMAL) {
            for (int sy = 0; sy < GRID_HEIGHT; sy++) {
                for (int sx = 0; sx < GRID_WIDTH; sx++) {
                    if (grid[sy][sx].selected && grid[sy][sx].specialType == NORMAL && grid[sy][sx].value > 0) {
                        value = grid[sy][sx].value;
                        break;
                    }
                }
                if (value > 0) break;
            }
            // If no regular dice found, can't merge
            if (value == 0) return 0;
        }
        
        int count = 0;
        
        // Flood fill to count connected tiles of same value that are selected
        std::vector<std::vector<bool>> visited(GRID_HEIGHT, std::vector<bool>(GRID_WIDTH, false));
        countConnected(x, y, value, visited, count);
        
        return count;
    }
    
    void countConnected(int x, int y, int value, std::vector<std::vector<bool>>& visited, int& count) {
        if (x < 0 || x >= GRID_WIDTH || y < 0 || y >= GRID_HEIGHT) return;
        if (visited[y][x] || !grid[y][x].selected) return;
        
        // Check if this tile can be part of the merge group
        bool canMerge = false;
        if (grid[y][x].specialType != NORMAL) {
            // Special dice can merge with any group
            canMerge = true;
        } else if (grid[y][x].value == value) {
            // Regular dice must match the value
            canMerge = true;
        }
        
        if (!canMerge) return;
        
        visited[y][x] = true;
        count++;
        
        // Check all 8 directions (orthogonal and diagonal)
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                if (dx == 0 && dy == 0) continue;
                countConnected(x + dx, y + dy, value, visited, count);
            }
        }
    }
    
    void performMerge(int targetX, int targetY) {
        int value = grid[targetY][targetX].value;
        
        // If target is special, find the regular dice value
        if (grid[targetY][targetX].specialType != NORMAL) {
            for (int sy = 0; sy < GRID_HEIGHT; sy++) {
                for (int sx = 0; sx < GRID_WIDTH; sx++) {
                    if (grid[sy][sx].selected && grid[sy][sx].specialType == NORMAL && grid[sy][sx].value > 0) {
                        value = grid[sy][sx].value;
                        break;
                    }
                }
                if (value > 0) break;
            }
        }
        
        int mergeCount = 0;
        int totalMergeBonus = 0;
        std::vector<SpecialDiceType> specialEffects;
        
        // Store target dice info before clearing
        int targetMergeCount = grid[targetY][targetX].mergeCount;
        
        // Count and clear all selected tiles (both regular and special) except target
        for (int y = 0; y < GRID_HEIGHT; y++) {
            for (int x = 0; x < GRID_WIDTH; x++) {
                if (grid[y][x].selected && !(x == targetX && y == targetY)) {
                    bool shouldClear = false;
                    
                    if (grid[y][x].specialType != NORMAL) {
                        // Special dice - collect effect and clear
                        specialEffects.push_back(grid[y][x].specialType);
                        shouldClear = true;
                    } else if (grid[y][x].value == value) {
                        // Regular dice of matching value
                        totalMergeBonus += grid[y][x].mergeCount;
                        shouldClear = true;
                    }
                    
                    if (shouldClear) {
                        grid[y][x].value = 0;
                        grid[y][x].specialType = NORMAL;
                        grid[y][x].selected = false;
                        mergeCount++;
                    }
                }
            }
        }
        
        // Add target dice's merge count and count it in the merge
        if (grid[targetY][targetX].specialType == NORMAL && grid[targetY][targetX].value == value) {
            totalMergeBonus += targetMergeCount;
        }
        mergeCount++; // Count the target dice
        
        // Calculate score
        // Base score: value * 10
        // Merge count bonus: x2 for 3 tiles, x3 for 4 tiles, etc.
        // Merge history bonus: +10% per total merge count
        int baseScore = value * 10;
        int mergeMultiplier = mergeCount;
        float mergeHistoryBonus = 1.0f + (totalMergeBonus * 0.1f);
        int earnedScore = static_cast<int>(baseScore * mergeMultiplier * mergeHistoryBonus);
        
        score += earnedScore;
        if (score > highScore) {
            highScore = score;
            saveHighScore();
        }
        
        // Place new tile at target location
        if (value < 6) {
            grid[targetY][targetX].value = value + 1;
            grid[targetY][targetX].mergeCount = totalMergeBonus + 1; // Inherit and increment merge counter
            grid[targetY][targetX].selected = false; // Clear selection
            grid[targetY][targetX].specialType = NORMAL; // Ensure it's normal
            
            
            // Start merge animation
            grid[targetY][targetX].anim.active = true;
            grid[targetY][targetX].anim.startTime = SDL_GetTicks();
            grid[targetY][targetX].anim.scale = 0.5f;
            grid[targetY][targetX].anim.glowIntensity = 1.0f;
            grid[targetY][targetX].anim.mergeCount = mergeCount;
            
            // Add floating text for value and score
            addFloatingText("+" + std::to_string(value + 1), targetX, targetY);
            addFloatingText("+" + std::to_string(earnedScore), targetX, targetY - 1);
        } else {
            // Handle 6s - explosive clear with bonus points
            grid[targetY][targetX].value = 0;
            grid[targetY][targetX].mergeCount = 0;
            
            // Bonus score for clearing 6s
            earnedScore *= 2;
            score += earnedScore;
            if (score > highScore) {
                highScore = score;
                saveHighScore();
            }
            
            addFloatingText("BOOM!", targetX, targetY);
            addFloatingText("+" + std::to_string(earnedScore), targetX, targetY - 1);
        }
        
        // Apply special dice effects
        applySpecialEffects(specialEffects, earnedScore);
        
        // Reset last selected position after merge
        lastSelectedX = -1;
        lastSelectedY = -1;
        
        // Make tiles fall down
        applyGravity();
        
        
        // Fill empty spaces from top
        fillEmptySpaces();
    }
    
    void applySpecialEffects(const std::vector<SpecialDiceType>& effects, int& earnedScore) {
        int bonusPoints = 0;
        
        for (SpecialDiceType effect : effects) {
            switch (effect) {
                case HEART:
                    // Remove lowest value dice from board
                    removeLowestDice();
                    addFloatingText("HEAL", 0, 1);
                    break;
                    
                case SKULL:
                    // Remove highest value dice from board (destructive)
                    removeHighestDice();
                    addFloatingText("SKULL", 1, 1);
                    break;
                    
                case FIST:
                    // Smash all adjacent dice around a random position
                    smashAdjacentDice();
                    addFloatingText("SMASH", 2, 1);
                    break;
                    
                case SWORD:
                    // Attack - remove highest value die and get bonus points
                    bonusPoints += removeHighestDice() * 10;
                    addFloatingText("ATTACK", 3, 1);
                    break;
                    
                case SHIELD:
                    // Protection - converts all 1s to 2s on the board
                    bonusPoints += protectLowDice();
                    addFloatingText("SHIELD", 4, 1);
                    break;
                    
                case SCROLL:
                    // Copy/duplicate a random high-value dice
                    copyRandomDice();
                    addFloatingText("COPY", 5, 1);
                    break;
                    
                case PLUS:
                    // Bonus points: +50 per plus die
                    bonusPoints += 50;
                    addFloatingText("+50", 0, 2);
                    break;
                    
                case MINUS:
                    // Reduce all dice values by 1 (minimum 1)
                    reduceAllDice();
                    addFloatingText("MINUS", 1, 2);
                    break;
                    
                case DIVIDE:
                    // Split highest value dice into two lower value dice
                    divideDice();
                    addFloatingText("DIVIDE", 2, 2);
                    break;
                    
                case MULTIPLY:
                    // Double the current earned score
                    earnedScore *= 2;
                    addFloatingText("x2!", 3, 2);
                    break;
                    
                case QUESTION:
                    // Random effect - trigger one of the other special dice effects
                    triggerRandomEffect(bonusPoints, earnedScore);
                    addFloatingText("???", 4, 2);
                    break;
                    
                case DOLLAR:
                    // Big money bonus: +100 points
                    bonusPoints += 100;
                    addFloatingText("$100", 5, 2);
                    break;
                    
                default:
                    break;
            }
        }
        
        // Add bonus points to total score
        if (bonusPoints > 0) {
            score += bonusPoints;
            
            // Update high score if needed
            if (score > highScore) {
                highScore = score;
                saveHighScore();
            }
        }
    }
    
    void removeLowestDice() {
        int lowestValue = 7;
        int targetX = -1, targetY = -1;
        
        // Find the lowest value dice on the board
        for (int y = 0; y < GRID_HEIGHT; y++) {
            for (int x = 0; x < GRID_WIDTH; x++) {
                if (grid[y][x].value > 0 && grid[y][x].value < lowestValue && grid[y][x].specialType == NORMAL) {
                    lowestValue = grid[y][x].value;
                    targetX = x;
                    targetY = y;
                }
            }
        }
        
        // Remove the found dice
        if (targetX >= 0 && targetY >= 0) {
            grid[targetY][targetX].value = 0;
            grid[targetY][targetX].specialType = NORMAL;
            grid[targetY][targetX].mergeCount = 0;
        }
    }
    
    int removeHighestDice() {
        int highestValue = 0;
        int targetX = -1, targetY = -1;
        
        // Find the highest value dice on the board
        for (int y = 0; y < GRID_HEIGHT; y++) {
            for (int x = 0; x < GRID_WIDTH; x++) {
                if (grid[y][x].value > highestValue && grid[y][x].specialType == NORMAL) {
                    highestValue = grid[y][x].value;
                    targetX = x;
                    targetY = y;
                }
            }
        }
        
        // Remove the found dice and return its value for bonus calculation
        if (targetX >= 0 && targetY >= 0) {
            grid[targetY][targetX].value = 0;
            grid[targetY][targetX].specialType = NORMAL;
            grid[targetY][targetX].mergeCount = 0;
            return highestValue;
        }
        
        return 0; // No dice found
    }
    
    void smashAdjacentDice() {
        // Find a random position and clear all adjacent dice
        int centerX = rng() % GRID_WIDTH;
        int centerY = rng() % GRID_HEIGHT;
        
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                int x = centerX + dx;
                int y = centerY + dy;
                if (x >= 0 && x < GRID_WIDTH && y >= 0 && y < GRID_HEIGHT) {
                    if (grid[y][x].specialType == NORMAL && grid[y][x].value > 0) {
                        grid[y][x].value = 0;
                        grid[y][x].mergeCount = 0;
                    }
                }
            }
        }
    }
    
    int protectLowDice() {
        int protectedCount = 0;
        for (int y = 0; y < GRID_HEIGHT; y++) {
            for (int x = 0; x < GRID_WIDTH; x++) {
                if (grid[y][x].specialType == NORMAL && grid[y][x].value == 1) {
                    grid[y][x].value = 2;
                    protectedCount++;
                }
            }
        }
        return protectedCount * 10; // 10 points per protected die
    }
    
    void copyRandomDice() {
        // Find highest value dice and try to duplicate it in an empty space
        int highestValue = 0;
        int sourceX = -1, sourceY = -1;
        
        // Find highest value regular dice
        for (int y = 0; y < GRID_HEIGHT; y++) {
            for (int x = 0; x < GRID_WIDTH; x++) {
                if (grid[y][x].specialType == NORMAL && grid[y][x].value > highestValue) {
                    highestValue = grid[y][x].value;
                    sourceX = x;
                    sourceY = y;
                }
            }
        }
        
        // Find empty space to place copy
        if (sourceX >= 0 && sourceY >= 0) {
            for (int y = 0; y < GRID_HEIGHT; y++) {
                for (int x = 0; x < GRID_WIDTH; x++) {
                    if (grid[y][x].value == 0 && grid[y][x].specialType == NORMAL) {
                        grid[y][x].value = highestValue;
                        grid[y][x].mergeCount = grid[sourceY][sourceX].mergeCount;
                        return;
                    }
                }
            }
        }
    }
    
    void reduceAllDice() {
        for (int y = 0; y < GRID_HEIGHT; y++) {
            for (int x = 0; x < GRID_WIDTH; x++) {
                if (grid[y][x].specialType == NORMAL && grid[y][x].value > 1) {
                    grid[y][x].value--;
                }
            }
        }
    }
    
    void divideDice() {
        // Find highest value dice and split it
        int highestValue = 0;
        int targetX = -1, targetY = -1;
        
        for (int y = 0; y < GRID_HEIGHT; y++) {
            for (int x = 0; x < GRID_WIDTH; x++) {
                if (grid[y][x].specialType == NORMAL && grid[y][x].value > highestValue && grid[y][x].value > 2) {
                    highestValue = grid[y][x].value;
                    targetX = x;
                    targetY = y;
                }
            }
        }
        
        if (targetX >= 0 && targetY >= 0) {
            int newValue = highestValue / 2;
            grid[targetY][targetX].value = newValue;
            
            // Try to place second half in adjacent empty space
            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    int x = targetX + dx;
                    int y = targetY + dy;
                    if (x >= 0 && x < GRID_WIDTH && y >= 0 && y < GRID_HEIGHT) {
                        if (grid[y][x].value == 0 && grid[y][x].specialType == NORMAL) {
                            grid[y][x].value = newValue;
                            return;
                        }
                    }
                }
            }
        }
    }
    
    void triggerRandomEffect(int& bonusPoints, int& earnedScore) {
        std::vector<SpecialDiceType> effects = {HEART, SKULL, FIST, SWORD, SHIELD, SCROLL, 
                                               PLUS, MINUS, DIVIDE, MULTIPLY, DOLLAR};
        SpecialDiceType randomEffect = effects[rng() % effects.size()];
        
        // Recursively trigger the random effect (but prevent infinite recursion)
        std::vector<SpecialDiceType> singleEffect = {randomEffect};
        if (randomEffect != QUESTION) { // Prevent question triggering another question
            applySpecialEffects(singleEffect, earnedScore);
        }
    }
    
    void applyGravity() {
        // Mark dice that need to fall and calculate their target positions
        for (int x = 0; x < GRID_WIDTH; x++) {
            int writePos = GRID_HEIGHT - 1;
            for (int y = GRID_HEIGHT - 1; y >= 0; y--) {
                if (grid[y][x].value != 0 || grid[y][x].specialType != NORMAL) {
                    if (y != writePos) {
                        // Start fall animation
                        grid[y][x].anim.falling = true;
                        grid[y][x].anim.fallStartY = y * (TILE_SIZE + TILE_MARGIN);
                        grid[y][x].anim.currentY = grid[y][x].anim.fallStartY;
                        grid[y][x].anim.rotationSpeed = 720.0f; // 2 rotations per second
                        grid[y][x].anim.startTime = SDL_GetTicks();
                        
                        // Move the dice data to new position
                        grid[writePos][x] = grid[y][x];
                        grid[y][x] = Tile(); // Reset to empty tile
                    }
                    writePos--;
                }
            }
        }
    }
    
    void fillEmptySpaces() {
        for (int x = 0; x < GRID_WIDTH; x++) {
            for (int y = 0; y < GRID_HEIGHT; y++) {
                if (grid[y][x].value == 0 && grid[y][x].specialType == NORMAL) {
                    int value = spawnNewValue();
                    if (value == -1) {
                        grid[y][x].value = 0; // Special dice have value 0
                        grid[y][x].specialType = getRandomSpecialType();
                    } else {
                        grid[y][x].value = value;
                        grid[y][x].specialType = NORMAL;
                    }
                }
            }
        }
    }
    
    void clearSelection() {
        for (int y = 0; y < GRID_HEIGHT; y++) {
            for (int x = 0; x < GRID_WIDTH; x++) {
                grid[y][x].selected = false;
            }
        }
        lastSelectedX = -1;
        lastSelectedY = -1;
    }
    
    void addFloatingText(const std::string& text, int gridX, int gridY) {
        for (auto& ft : floatingTexts) {
            if (!ft.active) {
                ft.text = text;
                ft.x = GRID_OFFSET_X + gridX * (TILE_SIZE + TILE_MARGIN) + TILE_SIZE / 2;
                ft.y = GRID_OFFSET_Y + gridY * (TILE_SIZE + TILE_MARGIN) + TILE_SIZE / 2;
                ft.alpha = 255.0f;
                ft.startTime = SDL_GetTicks();
                ft.active = true;
                break;
            }
        }
    }
    
    void updateAnimations() {
        Uint32 currentTime = SDL_GetTicks();
        
        // Update cursor smooth movement
        float cursorSpeed = 0.15f; // Interpolation speed (0-1, higher = faster)
        float deltaTime = 0.016f; // ~60fps
        
        // Lerp cursor position towards target
        float lerpFactor = 1.0f - pow(1.0f - cursorSpeed, deltaTime * 60.0f);
        cursorDisplayX += (cursorTargetX - cursorDisplayX) * lerpFactor;
        cursorDisplayY += (cursorTargetY - cursorDisplayY) * lerpFactor;
        
        // Snap to target if very close
        if (fabs(cursorDisplayX - cursorTargetX) < 0.1f) cursorDisplayX = cursorTargetX;
        if (fabs(cursorDisplayY - cursorTargetY) < 0.1f) cursorDisplayY = cursorTargetY;
        
        // Update tile animations
        for (int y = 0; y < GRID_HEIGHT; y++) {
            for (int x = 0; x < GRID_WIDTH; x++) {
                Animation& anim = grid[y][x].anim;
                
                // Handle falling animation
                if (anim.falling) {
                    float elapsed = (currentTime - anim.startTime) / 1000.0f;
                    float targetY = y * (TILE_SIZE + TILE_MARGIN);
                    float fallSpeed = 800.0f; // Pixels per second
                    float deltaTime = 0.016f; // ~60fps frame time
                    
                    // Update position
                    anim.currentY = std::min(targetY, anim.currentY + fallSpeed * deltaTime);
                    
                    // Update rotation
                    anim.rotation += anim.rotationSpeed * deltaTime;
                    
                    // Check if reached target
                    if (anim.currentY >= targetY) {
                        anim.currentY = targetY;
                        anim.falling = false;
                        anim.rotation = 0.0f; // Snap to upright position
                        // Add a small bounce effect
                        anim.active = true;
                        anim.scale = 0.8f;
                        anim.startTime = currentTime;
                    }
                }
                
                // Handle merge animations
                if (anim.active && !anim.falling) {
                    float elapsed = (currentTime - anim.startTime) / 1000.0f;
                    
                    // Scale animation (bounce effect)
                    if (elapsed < 0.3f) {
                        anim.scale = 0.5f + (elapsed / 0.3f) * 0.7f; // 0.5 to 1.2
                    } else if (elapsed < 0.5f) {
                        anim.scale = 1.2f - ((elapsed - 0.3f) / 0.2f) * 0.2f; // 1.2 to 1.0
                    } else {
                        anim.scale = 1.0f;
                    }
                    
                    // Glow fade out
                    anim.glowIntensity = std::max(0.0f, 1.0f - elapsed * 2.0f);
                    
                    // End animation after 1 second
                    if (elapsed > 1.0f) {
                        anim.active = false;
                    }
                }
            }
        }
        
        // Update floating texts
        for (auto& ft : floatingTexts) {
            if (ft.active) {
                float elapsed = (currentTime - ft.startTime) / 1000.0f;
                
                // Move up and fade out
                ft.y -= 60.0f * 0.016f; // Move up 60 pixels per second
                ft.alpha = std::max(0.0f, 255.0f * (1.0f - elapsed));
                
                // Deactivate after 1 second
                if (elapsed > 1.0f) {
                    ft.active = false;
                }
            }
        }
    }
    
    void renderScore() {
        // Draw score
        std::string scoreText = "Score: " + std::to_string(score);
        SDL_Color scoreColor = {255, 255, 255, 255};
        SDL_Surface* scoreSurface = TTF_RenderText_Solid(font, scoreText.c_str(), scoreColor);
        if (scoreSurface) {
            SDL_Texture* scoreTexture = SDL_CreateTextureFromSurface(renderer, scoreSurface);
            if (scoreTexture) {
                int w, h;
                SDL_QueryTexture(scoreTexture, NULL, NULL, &w, &h);
                SDL_Rect scoreRect = {20, 20, w, h};
                SDL_RenderCopy(renderer, scoreTexture, NULL, &scoreRect);
                SDL_DestroyTexture(scoreTexture);
            }
            SDL_FreeSurface(scoreSurface);
        }
        
        // Draw high score
        std::string highScoreText = "High Score: " + std::to_string(highScore);
        SDL_Color highScoreColor = {200, 200, 200, 255};
        SDL_Surface* highScoreSurface = TTF_RenderText_Solid(smallFont, highScoreText.c_str(), highScoreColor);
        if (highScoreSurface) {
            SDL_Texture* highScoreTexture = SDL_CreateTextureFromSurface(renderer, highScoreSurface);
            if (highScoreTexture) {
                int w, h;
                SDL_QueryTexture(highScoreTexture, NULL, NULL, &w, &h);
                SDL_Rect highScoreRect = {LOGICAL_WIDTH - w - 20, 20, w, h};
                SDL_RenderCopy(renderer, highScoreTexture, NULL, &highScoreRect);
                SDL_DestroyTexture(highScoreTexture);
            }
            SDL_FreeSurface(highScoreSurface);
        }
    }
    
    void render() {
        // Clear screen
        SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
        SDL_RenderClear(renderer);
        
        // Draw score at the top
        renderScore();
        
        // Draw grid
        for (int y = 0; y < GRID_HEIGHT; y++) {
            for (int x = 0; x < GRID_WIDTH; x++) {
                renderTile(x, y);
            }
        }
        
        // Draw cursor with smooth movement
        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
        
        // Use interpolated display position
        int displayX = GRID_OFFSET_X + static_cast<int>(cursorDisplayX) - 5;
        int displayY = GRID_OFFSET_Y + static_cast<int>(cursorDisplayY) - 5;
        
        SDL_Rect cursorRect = {
            displayX,
            displayY,
            TILE_SIZE + 10,
            TILE_SIZE + 10
        };
        
        // Draw main cursor border
        for (int i = 0; i < 3; i++) {
            SDL_Rect borderRect = {
                displayX - i,
                displayY - i,
                TILE_SIZE + 10 + 2 * i,
                TILE_SIZE + 10 + 2 * i
            };
            SDL_RenderDrawRect(renderer, &borderRect);
        }
        
        // Draw floating texts
        for (const auto& ft : floatingTexts) {
            if (ft.active) {
                SDL_Color textColor = {255, 255, 100, static_cast<Uint8>(ft.alpha)};
                SDL_Surface* surface = TTF_RenderText_Blended(smallFont, ft.text.c_str(), textColor);
                if (surface) {
                    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
                    if (texture) {
                        SDL_SetTextureAlphaMod(texture, static_cast<Uint8>(ft.alpha));
                        int w, h;
                        SDL_QueryTexture(texture, NULL, NULL, &w, &h);
                        SDL_Rect rect = {ft.x - w/2, static_cast<int>(ft.y) - h/2, w, h};
                        SDL_RenderCopy(renderer, texture, NULL, &rect);
                        SDL_DestroyTexture(texture);
                    }
                    SDL_FreeSurface(surface);
                }
            }
        }
        
        SDL_RenderPresent(renderer);
    }
    
    void renderTile(int x, int y) {
        const Animation& anim = grid[y][x].anim;
        float scale = anim.active ? anim.scale : 1.0f;
        
        int scaledSize = static_cast<int>(TILE_SIZE * scale);
        int offset = (TILE_SIZE - scaledSize) / 2;
        
        SDL_Rect tileRect = {
            GRID_OFFSET_X + x * (TILE_SIZE + TILE_MARGIN) + offset,
            GRID_OFFSET_Y + y * (TILE_SIZE + TILE_MARGIN) + offset,
            scaledSize,
            scaledSize
        };
        
        // Draw glow effect if animating
        if (anim.active && anim.glowIntensity > 0) {
            int glowSize = scaledSize + static_cast<int>(20 * anim.glowIntensity);
            int glowOffset = (scaledSize - glowSize) / 2;
            SDL_Rect glowRect = {
                tileRect.x + glowOffset,
                tileRect.y + glowOffset,
                glowSize,
                glowSize
            };
            Uint8 glowAlpha = static_cast<Uint8>(100 * anim.glowIntensity);
            SDL_SetRenderDrawColor(renderer, 255, 200, 100, glowAlpha);
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_RenderFillRect(renderer, &glowRect);
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
        }
        
        // Draw dice image
        if (grid[y][x].value > 0 || grid[y][x].specialType != NORMAL) {
            SDL_Texture* diceTexture = nullptr;
            
            if (grid[y][x].specialType != NORMAL) {
                // Use special dice texture
                auto it = specialDiceTextures.find(grid[y][x].specialType);
                if (it != specialDiceTextures.end()) {
                    diceTexture = it->second;
                }
            } else {
                // Use regular dice texture
                diceTexture = getDiceTexture(grid[y][x].value);
            }
            
            if (diceTexture) {
                // If falling, use animated Y position
                if (anim.falling) {
                    SDL_Rect animRect = tileRect;
                    animRect.y = GRID_OFFSET_Y + static_cast<int>(anim.currentY) + offset;
                    
                    // Render with rotation
                    SDL_Point center = {animRect.w / 2, animRect.h / 2};
                    SDL_RenderCopyEx(renderer, diceTexture, NULL, &animRect, 
                                   anim.rotation, &center, SDL_FLIP_NONE);
                } else if (fabs(anim.rotation) > 0.1f) {
                    // Still has rotation but not falling
                    SDL_Point center = {tileRect.w / 2, tileRect.h / 2};
                    SDL_RenderCopyEx(renderer, diceTexture, NULL, &tileRect, 
                                   anim.rotation, &center, SDL_FLIP_NONE);
                } else {
                    // Normal render
                    SDL_RenderCopy(renderer, diceTexture, NULL, &tileRect);
                }
            }
        }
        
        // Draw selection highlight (thick border)
        if (grid[y][x].selected) {
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); // Bright yellow
            // Draw thick border by drawing multiple rectangles
            for (int i = 0; i < 4; i++) {
                SDL_Rect borderRect = {
                    tileRect.x - i,
                    tileRect.y - i,
                    tileRect.w + 2 * i,
                    tileRect.h + 2 * i
                };
                SDL_RenderDrawRect(renderer, &borderRect);
            }
        }
            
        // Draw merge count indicator (only for regular dice that have been merged)
        if (grid[y][x].value > 0 && grid[y][x].mergeCount > 0 && grid[y][x].specialType == NORMAL) {
            std::string mergeText = "x" + std::to_string(grid[y][x].mergeCount);
            SDL_Color mergeColor = {255, 255, 0, 255}; // Bright yellow text
            SDL_Surface* mergeSurface = TTF_RenderText_Solid(smallFont, mergeText.c_str(), mergeColor);
            if (mergeSurface) {
                SDL_Texture* mergeTexture = SDL_CreateTextureFromSurface(renderer, mergeSurface);
                if (mergeTexture) {
                    int mw, mh;
                    SDL_QueryTexture(mergeTexture, NULL, NULL, &mw, &mh);
                    
                    // Draw dark background for better visibility
                    SDL_Rect bgRect = {
                        tileRect.x + scaledSize - mw - 8,
                        tileRect.y + scaledSize - mh - 8,
                        mw + 6,
                        mh + 6
                    };
                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
                    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
                    SDL_RenderFillRect(renderer, &bgRect);
                    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
                    
                    // Draw the text
                    SDL_Rect mergeRect = {
                        tileRect.x + scaledSize - mw - 5,
                        tileRect.y + scaledSize - mh - 5,
                        mw,
                        mh
                    };
                    SDL_RenderCopy(renderer, mergeTexture, NULL, &mergeRect);
                    SDL_DestroyTexture(mergeTexture);
                }
                SDL_FreeSurface(mergeSurface);
            }
        }
    }
    
    void cleanup() {
        // Clean up dice textures
        for (auto& pair : diceTextures) {
            SDL_DestroyTexture(pair.second);
        }
        diceTextures.clear();
        
        // Clean up special dice textures
        for (auto& pair : specialDiceTextures) {
            SDL_DestroyTexture(pair.second);
        }
        specialDiceTextures.clear();
        
        if (font) {
            TTF_CloseFont(font);
            font = nullptr;
        }
        if (smallFont) {
            TTF_CloseFont(smallFont);
            smallFont = nullptr;
        }
        if (renderer) {
            SDL_DestroyRenderer(renderer);
            renderer = nullptr;
        }
        if (window) {
            SDL_DestroyWindow(window);
            window = nullptr;
        }
        
        IMG_Quit();
        TTF_Quit();
        SDL_Quit();
    }
};

int main(int argc, char* argv[]) {
    Game game;
    
    if (!game.init()) {
        std::cerr << "Failed to initialize game!" << std::endl;
        return 1;
    }
    
    game.run();
    
    return 0;
}