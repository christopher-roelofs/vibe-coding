#ifndef GAME_H
#define GAME_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <box2d/box2d.h>
#include <memory>
#include <string>
#include <vector>
#include <filesystem>

#include "constants.h"
#include "Level.h"
#include "Maze.h"
#include "Ball.h"
#include "ReverseItem.h"
#include "Warp.h"

// Define game states
enum class GameState {
    START_SCREEN,
    LEVEL_INTRO,
    GAMEPLAY,
    LEVEL_COMPLETE
};

// Structure to hold level pack metadata
struct LevelPackInfo {
    std::string filepath;
    std::string name;
    std::string description;
    std::string author;
    std::string date;
};

class Game {
public:
    Game();
    ~Game();

    bool init();
    bool loadLevel(const std::string& level_filepath);
    void run();
    
    // New methods for level pack selection
    void loadLevelPacks();
    bool loadSelectedLevelPack();

private:
    // Helper function to draw a circle
    static void SDL_RenderDrawCircle(SDL_Renderer* renderer, int center_x, int center_y, int radius);
    
    void processInput();
    void update(float delta_time);
    void render();
    void cleanup();
    
    // New methods for different game states
    void processStartScreenInput();
    void updateStartScreen(float delta_time);
    void renderStartScreen();

    void processLevelIntroInput();
    void updateLevelIntro(float delta_time);
    void renderLevelIntro();

    void processLevelCompleteInput();
    void updateLevelComplete(float delta_time);
    void renderLevelComplete();
    
    void processGameplayInput();
    void updateGameplay(float delta_time);
    void renderGameplay();
    
    // Helper method to create or recreate the maze and ball based on current level
    void createMazeAndBall();
    
    // Helper method to update camera offsets based on maze size
    void updateCameraOffsets();

    // Helper for rendering Box2D bodies (e.g., for debugging or simple shapes)
    void renderBox2DBody(SDL_Renderer* renderer, b2BodyId bodyId, SDL_Color color, float camera_offset_x, float camera_offset_y);

    SDL_Window* window_;
    SDL_Renderer* renderer_;
    bool is_running_;
    Uint32 last_tick_;


    b2WorldId worldId_;
    float time_accumulator_ = 0.0f;

    std::unique_ptr<Level> current_level_;
    std::unique_ptr<Maze> maze_;
    std::unique_ptr<Ball> ball_;
    
    // Game objects
    std::vector<std::unique_ptr<ReverseItem>> reverse_items_;
    std::vector<std::unique_ptr<Warp>> warps_;
    
    // For warp cooldown to prevent immediate re-triggering
    bool is_warp_cooldown_ = false;
    float warp_cooldown_timer_ = 0.0f;
    static constexpr float WARP_COOLDOWN_TIME = 1.5f; // seconds
    bool is_level_won_ = false;
    
    // Control inversion state
    bool controls_inverted_ = false;
    float reverse_effect_timer_ = 0.0f;
    const float REVERSE_EFFECT_DURATION = 5.0f; // 5 seconds

    b2Vec2 maze_world_origin_meters_; // To store maze origin in world coordinates

    // Camera/View
    float camera_offset_x_ = 0.0f;
    float camera_offset_y_ = 0.0f;
    
    // Game state management
    GameState current_state_ = GameState::START_SCREEN;
    bool just_started_gameplay_ = false; // Added to manage first-frame input
    
    // Level pack selection
    std::vector<LevelPackInfo> level_packs_;
    size_t current_level_pack_index_ = 0;
    
    // Font for rendering text
    TTF_Font* font_ = nullptr;
    TTF_Font* title_font_ = nullptr;
};

#endif // GAME_H
