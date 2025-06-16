#include "Game.h"
#include <SDL2/SDL_ttf.h> // For TTF_Quit
#include <iostream>
#include <fstream> // For std::ifstream
#include <cmath> // For M_PI, b2DistanceSquared

// Implementation of the circle drawing helper function
void Game::SDL_RenderDrawCircle(SDL_Renderer* renderer, int center_x, int center_y, int radius) {
    // Using the midpoint circle algorithm
    int x = radius;
    int y = 0;
    int err = 0;
    
    while (x >= y) {
        SDL_RenderDrawPoint(renderer, center_x + x, center_y + y);
        SDL_RenderDrawPoint(renderer, center_x + y, center_y + x);
        SDL_RenderDrawPoint(renderer, center_x - y, center_y + x);
        SDL_RenderDrawPoint(renderer, center_x - x, center_y + y);
        SDL_RenderDrawPoint(renderer, center_x - x, center_y - y);
        SDL_RenderDrawPoint(renderer, center_x - y, center_y - x);
        SDL_RenderDrawPoint(renderer, center_x + y, center_y - x);
        SDL_RenderDrawPoint(renderer, center_x + x, center_y - y);
        
        if (err <= 0) {
            y += 1;
            err += 2*y + 1;
        }
        if (err > 0) {
            x -= 1;
            err -= 2*x + 1;
        }
    }
}

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Game::Game() :
    window_(nullptr),
    renderer_(nullptr),
    is_running_(false),
    last_tick_(0),
    worldId_(b2_nullWorldId),
    current_level_(nullptr),
    maze_(nullptr),
    ball_(nullptr),
    maze_world_origin_meters_({0.0f, 0.0f}),
    font_(nullptr),
    title_font_(nullptr)
{
    // In-class initializers handle the rest
}

Game::~Game() {
    cleanup();
}

bool Game::init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // Initialize SDL_ttf
    if (TTF_Init() < 0) {
        std::cerr << "SDL_ttf could not initialize! TTF_Error: " << TTF_GetError() << std::endl;
        return false;
    }

    window_ = SDL_CreateWindow("Ball Maze Game (Box2D 3.x)", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
                               SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window_) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer_) {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }



    // Load fonts
    font_ = TTF_OpenFont("assets/fonts/DejaVuSans.ttf", 24);
    if (!font_) {
        std::cerr << "Failed to load font! TTF_Error: " << TTF_GetError() << std::endl;
        return false;
    }
    
    title_font_ = TTF_OpenFont("assets/fonts/DejaVuSans.ttf", 36);
    if (!title_font_) {
        std::cerr << "Failed to load title font! TTF_Error: " << TTF_GetError() << std::endl;
        return false;
    }

    // Initialize Box2D world with increased gravity for faster gameplay
    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = {0.0f, 20.0f}; // Increased gravity for faster movement
    worldId_ = b2CreateWorld(&worldDef);
    if (!b2World_IsValid(worldId_)) {
        std::cerr << "Box2D world could not be created!" << std::endl;
        return false;
    }

    is_running_ = true;
    last_tick_ = SDL_GetTicks();
    return true;
}

void Game::loadLevelPacks() {
    level_packs_.clear();
    current_level_pack_index_ = 0;
    
    // Check if directory exists
    const std::string levels_dir = "assets/levels";
    try {
        // Iterate through all files in the levels directory
        for (const auto& entry : std::filesystem::directory_iterator(levels_dir)) {
            if (entry.is_regular_file() && entry.path().extension() == ".txt") {
                std::string filepath = entry.path().string();
                
                // Open the file and read the level pack metadata
                std::ifstream file(filepath);
                if (file.is_open()) {
                    LevelPackInfo pack_info;
                    pack_info.filepath = filepath;
                    
                    // Read only the top metadata section (before any level sections)
                    bool in_pack_metadata = true;
                    bool found_empty_line = false;
                    std::string line;
                    
                    while (std::getline(file, line) && in_pack_metadata) {
                        // Check for empty lines
                        if (line.empty()) {
                            // If we've already found pack metadata and now hit an empty line,
                            // this might be the separator before level sections
                            if (!pack_info.name.empty()) {
                                found_empty_line = true;
                            }
                            continue;
                        }
                        
                        // If we found an empty line and now a non-empty line, check if it's a new section
                        if (found_empty_line && line[0] == ';') {
                            // This is likely the start of a level section
                            in_pack_metadata = false;
                            continue;
                        }
                        
                        // Look for metadata in comment lines
                        if (line[0] == ';') {
                            std::string metadata = line.substr(1);
                            // Trim leading whitespace
                            metadata.erase(0, metadata.find_first_not_of(" \t"));
                            
                            if (metadata.find("Name: ") == 0) {
                                pack_info.name = metadata.substr(6);
                            } else if (metadata.find("Description: ") == 0) {
                                pack_info.description = metadata.substr(13);
                            } else if (metadata.find("Author: ") == 0) {
                                pack_info.author = metadata.substr(8);
                            } else if (metadata.find("Date: ") == 0) {
                                pack_info.date = metadata.substr(6);
                            }
                        } else {
                            // Non-comment line - end of metadata
                            in_pack_metadata = false;
                        }
                    }
                    
                    // Add to level packs if we found a name
                    if (!pack_info.name.empty()) {
                        level_packs_.push_back(pack_info);
                        std::cout << "Found level pack: " << pack_info.name << " at " << filepath << std::endl;
                        std::cout << "  Description: '" << pack_info.description << "'" << std::endl;
                        std::cout << "  Author: '" << pack_info.author << "'" << std::endl;
                        std::cout << "  Date: '" << pack_info.date << "'" << std::endl;
                    }
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error scanning level packs: " << e.what() << std::endl;
    }
    
    std::cout << "Found " << level_packs_.size() << " level packs" << std::endl;
}

bool Game::loadSelectedLevelPack() {
    if (level_packs_.empty()) {
        std::cerr << "No level packs available" << std::endl;
        return false;
    }
    
    const auto& selected_pack = level_packs_[current_level_pack_index_];
    return loadLevel(selected_pack.filepath);
}

bool Game::loadLevel(const std::string& level_filepath) {
    current_level_ = std::make_unique<Level>();
    if (!current_level_->loadFromFile(level_filepath)) {
        std::cerr << "Failed to load level data from: " << level_filepath << std::endl;
        return false;
    }

    // Define maze origin in world coordinates (e.g., top-left of maze area)
    // Let's place it slightly offset from the screen corner for visibility
    float maze_world_origin_x_meters = (SCREEN_WIDTH * 0.1f) / PPM;
    float maze_world_origin_y_meters = (SCREEN_HEIGHT * 0.1f) / PPM;
    maze_world_origin_meters_ = {maze_world_origin_x_meters, maze_world_origin_y_meters};

    // Create the maze and ball for the current level
    createMazeAndBall();

    // Adjust camera to center the maze initially, if maze is smaller than screen
    // For simplicity, camera is static for now. Can be improved later.
    // camera_offset_x_ = SCREEN_WIDTH / 2.0f - maze_->getMazeCenterMeters().x * PPM;
    // camera_offset_y_ = SCREEN_HEIGHT / 2.0f - maze_->getMazeCenterMeters().y * PPM;

    return true;
}

void Game::run() {
    // Load level packs at startup
    loadLevelPacks();

    const int TARGET_FPS = 60;
    const Uint32 TARGET_FRAME_TIME_MS = 1000 / TARGET_FPS;

    // Initialize last_tick_ properly before the loop for the first delta_time calculation
    last_tick_ = SDL_GetTicks(); 

    while (is_running_) {
        Uint32 frame_start_time = SDL_GetTicks();

        // Calculate delta_time for the current frame
        float delta_time = (frame_start_time - last_tick_) / 1000.0f;
        last_tick_ = frame_start_time; // Update last_tick_ for the next frame

        // Clamp delta_time to prevent large jumps if game pauses/lags (e.g. > 0.05s)
        if (delta_time > 0.05f) {
            delta_time = 0.05f;
        }

        processInput();
        update(delta_time); // Physics and game logic update
        render();           // Drawing

        Uint32 frame_processing_time = SDL_GetTicks() - frame_start_time;

        if (frame_processing_time < TARGET_FRAME_TIME_MS) {
            SDL_Delay(TARGET_FRAME_TIME_MS - frame_processing_time);
        }
    }
}

// Track key states
static bool left_key_pressed = false;
static bool right_key_pressed = false;

void Game::processInput() {
    switch (current_state_) {
        case GameState::START_SCREEN:
            processStartScreenInput();
            break;
        case GameState::LEVEL_INTRO:
            processLevelIntroInput();
            break;
        case GameState::LEVEL_COMPLETE:
            processLevelCompleteInput();
            break;
        case GameState::GAMEPLAY:
            processGameplayInput();
            break;
    }
}

void Game::processStartScreenInput() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            is_running_ = false;
        }
        
        if (event.type == SDL_KEYDOWN) {
            switch (event.key.keysym.sym) {
                case SDLK_ESCAPE:
                    is_running_ = false;
                    break;
                    
                case SDLK_LEFT:
                    if (current_level_pack_index_ > 0) {
                        current_level_pack_index_--;
                    } else if (!level_packs_.empty()) {
                        current_level_pack_index_ = level_packs_.size() - 1;
                    }
                    break;
                    
                case SDLK_RIGHT:
                    if (!level_packs_.empty()) {
                        current_level_pack_index_ = (current_level_pack_index_ + 1) % level_packs_.size();
                    }
                    break;
                    
                case SDLK_RETURN:
                    if (!level_packs_.empty()) {
                        if (loadSelectedLevelPack()) {
                            current_state_ = GameState::LEVEL_INTRO;
                        }
                    }
                    break;
            }
        }
    }
}

void Game::processGameplayInput() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            is_running_ = false;
        }
        
        if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
            bool key_pressed = (event.type == SDL_KEYDOWN);
            
            switch (event.key.keysym.sym) {
                case SDLK_ESCAPE:
                    if (key_pressed) {
                        // Return to start screen
                        current_state_ = GameState::START_SCREEN;
                        // Clean up gameplay resources
                        if (ball_) ball_.reset();
                        if (maze_) maze_.reset();
                        reverse_items_.clear();
                        warps_.clear();
                    }
                    break;
                    
                case SDLK_LEFT:
                    left_key_pressed = key_pressed;
                    break;
                    
                case SDLK_RIGHT:
                    right_key_pressed = key_pressed;
                    break;
                    
                case SDLK_r:
                    if (key_pressed && ball_ && current_level_ && maze_) {
                        b2Vec2 ball_start_grid_pos = current_level_->getBallStartPosition();
                        float ball_start_x_meters = maze_world_origin_meters_.x + (ball_start_grid_pos.x + 0.5f) * (TILE_SIZE / PPM);
                        float ball_start_y_meters = maze_world_origin_meters_.y + (ball_start_grid_pos.y + 0.5f) * (TILE_SIZE / PPM);
                        ball_->reset({ball_start_x_meters, ball_start_y_meters});
                        maze_->resetRotation();
                        
                        // Reset warp positions to their original positions
                        for (auto& warp : warps_) {
                            if (warp && b2Body_IsValid(warp->getBodyId())) {
                                b2Vec2 original_pos = warp->getOriginalPosition();
                                b2Rot identity_rot = {1.0f, 0.0f}; // Identity rotation (angle = 0)
                                b2Body_SetTransform(warp->getBodyId(), original_pos, identity_rot);
                            }
                        }
                    }
                    break;
            }
        }
    }
    
    // Handle continuous rotation based on key states
    if (maze_) {
        if (just_started_gameplay_) {
            // For the very first frame of gameplay, ensure no rotation command is set,
            // effectively ignoring any held-down rotation keys from the transition.
            maze_->setRotationDirection(0);
            just_started_gameplay_ = false; // Reset flag for subsequent frames
        } else {
            int rotation_direction = 0;
            if (left_key_pressed && !right_key_pressed) {
                // Left key: counter-clockwise by default, clockwise if inverted
                rotation_direction = controls_inverted_ ? 1 : -1;
            } else if (right_key_pressed && !left_key_pressed) {
                // Right key: clockwise by default, counter-clockwise if inverted
                rotation_direction = controls_inverted_ ? -1 : 1;
            } else {
                // No keys pressed or both pressed, stop rotation
                rotation_direction = 0;
            }
            maze_->setRotationDirection(rotation_direction);
        }
    }
}

void Game::update(float delta_time) {
    switch (current_state_) {
        case GameState::START_SCREEN:
            updateStartScreen(delta_time);
            break;
        case GameState::LEVEL_INTRO:
            updateLevelIntro(delta_time);
            break;
        case GameState::LEVEL_COMPLETE:
            updateLevelComplete(delta_time);
            break;
        case GameState::GAMEPLAY:
            updateGameplay(delta_time);
            break;
    }
}

void Game::updateStartScreen(float delta_time) {
    // Nothing to update in the start screen for now
}

void Game::updateGameplay(float delta_time) {
    // Accumulate time for fixed timestep
    time_accumulator_ += delta_time;
    
    // Handle warp cooldown timer
    if (is_warp_cooldown_) {
        warp_cooldown_timer_ -= delta_time;
        if (warp_cooldown_timer_ <= 0.0f) {
            is_warp_cooldown_ = false;
        }
    }
    
    // Handle reverse effect timer
    if (controls_inverted_) {
        reverse_effect_timer_ -= delta_time;
        if (reverse_effect_timer_ <= 0.0f) {
            controls_inverted_ = false;
            std::cout << "Controls returned to normal" << std::endl;
        }
    }

    // Update cooldowns for all reverse items
    for (const auto& item : reverse_items_) {
        if (item) {
            item->updateCooldown(delta_time);
        }
    }
    
    // Update the maze (handles rotation animation)
    if (maze_) {
        maze_->update(delta_time);
        
        // If the maze is rotating, update warp positions to rotate with the maze
        if (maze_->isRotating()) {
            b2Vec2 maze_center = maze_->getMazeCenterWorldCoords();
            float current_rotation = maze_->getCurrentRotationRad();
            
            // Rotate all warp points around the maze center
            for (auto& warp : warps_) {
                b2BodyId warp_body = warp->getBodyId();
                if (b2Body_IsValid(warp_body)) {
                    // Get the warp's original position relative to maze center
                    b2Vec2 original_pos = warp->getOriginalPosition();
                    b2Vec2 relative_pos = b2Sub(original_pos, maze_center);
                    
                    // Apply rotation
                    b2Rot rotation = b2MakeRot(current_rotation);
                    b2Vec2 rotated_pos = b2Add(maze_center, b2RotateVector(rotation, relative_pos));
                    
                    // Update warp position
                    b2Body_SetTransform(warp_body, rotated_pos, rotation);
                }
            }
        }
    }
    
    // Check for warp collisions
    if (ball_) {
        b2BodyId ball_body = ball_->getBodyId();
        b2Vec2 ball_pos = b2Body_GetPosition(ball_body);
        
        // Check against all warps
        for (const auto& warp : warps_) {
            // Skip warps that are on cooldown
            if (warp->isOnCooldown()) {
                continue;
            }
            
            b2BodyId warp_body = warp->getBodyId();
            b2Vec2 warp_pos = b2Body_GetPosition(warp_body);
            
            // Simple distance check first
            b2Vec2 delta = b2Sub(ball_pos, warp_pos);
            float distance_squared = b2Dot(delta, delta);
            float min_distance = ball_->getRadius() + warp->getRadius();
            
            // Debug output for collision detection
            if (distance_squared < min_distance * min_distance * 1.5f) {
                std::cout << "Ball near warp ID " << warp->getId() 
                          << ", distance: " << sqrt(distance_squared)
                          << ", threshold: " << min_distance * 0.8f << std::endl;
            }
            
            // For Box2D 3.x, we'll use a simple distance check
            // If the distance is less than the sum of radii, consider it a collision
            if (distance_squared < min_distance * min_distance * 0.7f) { // Reduced threshold for more reliable detection
                std::cout << "Warp collision detected with ID " << warp->getId() << std::endl;
                Warp::handleWarpCollision(warp->getId(), ball_body, warps_);
                break; // Only process one warp collision per frame
            }
        }
    }
    
    // Fixed time step for physics, using TIME_STEP from constants.h (1/120s)
    // time_accumulator_ was already incremented at the start of updateGameplay
    
    // Limit the number of steps to avoid spiral of death
    const int MAX_PHYSICS_STEPS = 4;
    int steps_taken = 0;
    
    // Perform fixed time steps
    while (time_accumulator_ >= TIME_STEP && steps_taken < MAX_PHYSICS_STEPS) {
        // Step the physics world - Box2D will automatically apply angular velocity to the maze
        // and handle collisions continuously
        b2World_Step(worldId_, TIME_STEP, POSITION_ITERATIONS); // POSITION_ITERATIONS used as subStepCount
        
        time_accumulator_ -= TIME_STEP;
        steps_taken++;
    }

    // The global reverse_effect_timer_ (lines 401-407) handles when controls return to normal.
    // Individual ReverseItem cooldowns are handled by their own updateCooldown methods.
    // The old logic for resetting all items when the global effect timer ends is removed.
    
    // For large levels, continuously update camera to follow the ball
    if (current_level_ && ball_) {
        int grid_width = current_level_->getWidth();
        int grid_height = current_level_->getHeight();
        float maze_width_pixels = grid_width * TILE_SIZE;
        float maze_height_pixels = grid_height * TILE_SIZE;
        
        if (maze_width_pixels > SCREEN_WIDTH || maze_height_pixels > SCREEN_HEIGHT) {
            updateCameraOffsets();
        }
    }
    
    // Check for collisions with reverse items
    if (ball_ && maze_) {
        b2Vec2 ball_pos = ball_->getPosition();
        float ball_radius = ball_->getRadius();
        float maze_rotation = maze_->getCurrentRotationRad();
        b2Vec2 maze_pivot_point = maze_->getMazeCenterWorldCoords();
        // ball_pos and ball_radius are already defined in this scope if ball_ is valid

        for (const auto& reverse_item : reverse_items_) {
            if (reverse_item && reverse_item->isActive() && !reverse_item->isCoolingDown()) {
                // Get the original position of the reverse item
                b2Vec2 item_pos = reverse_item->getPosition();
                
                // Calculate the rotated position based on maze rotation
                b2Vec2 offset_from_pivot = item_pos - maze_pivot_point;
                float rotated_offset_x = offset_from_pivot.x * cosf(maze_rotation) - offset_from_pivot.y * sinf(maze_rotation);
                float rotated_offset_y = offset_from_pivot.x * sinf(maze_rotation) + offset_from_pivot.y * cosf(maze_rotation);
                b2Vec2 rotated_item_pos = maze_pivot_point + b2Vec2{rotated_offset_x, rotated_offset_y};
                
                // Use the rotated position for collision detection
                // ball_pos and ball_radius are already available from the outer if (ball_ && maze_)
                float dx = ball_pos.x - rotated_item_pos.x;
                float dy = ball_pos.y - rotated_item_pos.y;
                float distance_squared = dx * dx + dy * dy;
                
                float item_radius = reverse_item->getRadius(); // Using new method
                float collision_threshold_sq = (ball_radius + item_radius) * (ball_radius + item_radius);

                if (distance_squared < collision_threshold_sq) {
                    // Invert controls
                    controls_inverted_ = true;
                    reverse_effect_timer_ = REVERSE_EFFECT_DURATION;
                    
                    // Start the cooldown for the specific item hit
                    reverse_item->startCooldown();
                    
                    // Visual or audio feedback could be added here
                    std::cout << "Controls inverted for " << REVERSE_EFFECT_DURATION << " seconds!" << std::endl;
                }
            }
        }
    }
    
    if (steps_taken > 0) { // For debugging
        //std::cout << "Physics steps taken this frame: " << steps_taken;
        if (ball_) {
            // Using PPM to see screen-comparable values, though raw meters are fine too
            // The line referencing ball_pos has been removed as ball_pos is no longer defined here.
        }
        //std::cout << std::endl;
    }

    // Check for goal collision
    if (ball_ && !is_level_won_ && current_level_ && maze_) {
        const LevelData* level_data = current_level_->getCurrentLevelData();
        if (level_data) {
            // Calculate current goal position based on maze rotation
            b2Vec2 goal_pos_grid = level_data->goal_position;
            float maze_rotation = maze_->getCurrentRotationRad();
            b2Vec2 maze_pivot_point = maze_->getMazeCenterWorldCoords();

            b2Vec2 initial_goal_pos_meters = {
                maze_world_origin_meters_.x + (goal_pos_grid.x + 0.5f) * (TILE_SIZE / PPM),
                maze_world_origin_meters_.y + (goal_pos_grid.y + 0.5f) * (TILE_SIZE / PPM)
            };

            b2Vec2 offset_from_pivot = initial_goal_pos_meters - maze_pivot_point;
            float rotated_offset_x = offset_from_pivot.x * cosf(maze_rotation) - offset_from_pivot.y * sinf(maze_rotation);
            float rotated_offset_y = offset_from_pivot.x * sinf(maze_rotation) + offset_from_pivot.y * cosf(maze_rotation);
            b2Vec2 current_goal_pos_meters = maze_pivot_point + b2Vec2{rotated_offset_x, rotated_offset_y};
            
            // Now check for collision
            b2Vec2 ball_pos = ball_->getPosition();
            float goal_radius_meters = (TILE_SIZE / PPM) * 0.5f;
            float distance_sq = b2DistanceSquared(ball_pos, current_goal_pos_meters);
            float total_radius = ball_->getRadius() + goal_radius_meters;
            float total_radius_sq = total_radius * total_radius;

            if (distance_sq < total_radius_sq) {
                std::cout << "Congratulations! You completed level " << (current_level_->getCurrentLevelIndex() + 1)
                          << " of " << current_level_->getTotalLevels() << "!" << std::endl;
                if (maze_) { // Explicitly stop maze rotation before state change
                    maze_->setRotationDirection(0);
                }
                is_level_won_ = true;
                current_state_ = GameState::LEVEL_COMPLETE;
                return; // Exit updateGameplay to show complete screen immediately
            }
        }
    }
    // Check hole condition
    if (ball_ && current_level_ && maze_ && maze_world_origin_meters_.x != 0.0f) { // Check maze_world_origin_meters_ to ensure it's initialized
        const auto& hole_grid_positions = current_level_->getHolePositions();
        float hole_radius_meters = (TILE_SIZE / PPM) * 0.45f; // Collision radius for holes (slightly smaller to prevent overlap)

        float maze_rotation = maze_->getCurrentRotationRad(); // Get current maze rotation
        b2Vec2 maze_pivot_point = maze_->getMazeCenterWorldCoords(); // Get maze pivot
        
        for (const auto& grid_pos : hole_grid_positions) {
            b2Vec2 initial_hole_world_pos = {
                maze_world_origin_meters_.x + (grid_pos.x + 0.5f) * (TILE_SIZE / PPM),
                maze_world_origin_meters_.y + (grid_pos.y + 0.5f) * (TILE_SIZE / PPM)
            };
            b2Vec2 offset_from_pivot_hole = initial_hole_world_pos - maze_pivot_point;
            float rotated_offset_x_hole = offset_from_pivot_hole.x * cosf(maze_rotation) - offset_from_pivot_hole.y * sinf(maze_rotation);
            float rotated_offset_y_hole = offset_from_pivot_hole.x * sinf(maze_rotation) + offset_from_pivot_hole.y * cosf(maze_rotation);
            b2Vec2 current_hole_center_meters = maze_pivot_point + b2Vec2{rotated_offset_x_hole, rotated_offset_y_hole};

            b2Vec2 ball_pos = ball_->getPosition();
            float distance_sq = b2DistanceSquared(ball_pos, current_hole_center_meters);

            if (distance_sq < (hole_radius_meters * hole_radius_meters)) {
                std::cout << "Fell into a hole!" << std::endl;
                
                b2Vec2 ball_start_grid_pos = current_level_->getBallStartPosition();
                float ball_start_x_meters = maze_world_origin_meters_.x + (ball_start_grid_pos.x + 0.5f) * (TILE_SIZE / PPM);
                float ball_start_y_meters = maze_world_origin_meters_.y + (ball_start_grid_pos.y + 0.5f) * (TILE_SIZE / PPM);
                ball_->reset({ball_start_x_meters, ball_start_y_meters});
                if (maze_) {
                    maze_->resetRotation();
                }
                
                // Reset reverse controls effect
                controls_inverted_ = false;
                reverse_effect_timer_ = 0.0f;
                // Reverse items are reset by createMazeAndBall() when the level restarts.
                // No explicit loop needed here as their cooldowns are independent.
                
                // Reset warp positions to their original positions
                for (auto& warp : warps_) {
                    if (warp && b2Body_IsValid(warp->getBodyId())) {
                        b2Vec2 original_pos = warp->getOriginalPosition();
                        b2Rot identity_rot = {1.0f, 0.0f}; // Identity rotation (angle = 0)
                        b2Body_SetTransform(warp->getBodyId(), original_pos, identity_rot);
                    }
                }
                
                std::cout << "Level reset due to falling into a hole." << std::endl;
                break; 
            }
        }
    }
}

void Game::render() {
    SDL_SetRenderDrawColor(renderer_, 30, 30, 50, 255); // Dark blue background
    SDL_RenderClear(renderer_);
    
    switch (current_state_) {
        case GameState::START_SCREEN:
            renderStartScreen();
            break;
        case GameState::LEVEL_INTRO:
            renderLevelIntro();
            break;
        case GameState::LEVEL_COMPLETE:
            renderLevelComplete();
            break;
        case GameState::GAMEPLAY:
            renderGameplay();
            break;
    }
    
    SDL_RenderPresent(renderer_);
}

void Game::processLevelIntroInput() {
    SDL_Event e;
    while (SDL_PollEvent(&e) != 0) {
        if (e.type == SDL_QUIT) {
            is_running_ = false;
        } else if (e.type == SDL_KEYDOWN) {
            if (e.key.keysym.sym == SDLK_RETURN) {
                current_state_ = GameState::GAMEPLAY;
                createMazeAndBall(); // Create maze & ball for the gameplay session
                // Reset persistent key states to prevent rotation carry-over
                left_key_pressed = false;
                right_key_pressed = false;
                if (maze_) { // Ensure the new maze starts with no rotation command
                    maze_->setRotationDirection(0);
                }
                just_started_gameplay_ = true; // Set flag to ignore input for the first gameplay frame
            }
        }
    }
}

void Game::updateLevelIntro(float delta_time) {
    // Nothing to update for now
}

void Game::processLevelCompleteInput() {
    SDL_Event e;
    while (SDL_PollEvent(&e) != 0) {
        if (e.type == SDL_QUIT) {
            is_running_ = false;
        } else if (e.type == SDL_KEYDOWN) {
            switch (e.key.keysym.sym) {
                case SDLK_RETURN:
                    if (current_level_->loadNextLevel()) {
                        // Maze and ball will be created when transitioning from LEVEL_INTRO to GAMEPLAY
                        // Show the intro screen for the new level
                        current_state_ = GameState::LEVEL_INTRO;
                    } else {
                        // No more levels, return to start screen
                        current_state_ = GameState::START_SCREEN;
                    }
                    break;
                case SDLK_ESCAPE:
                    current_state_ = GameState::START_SCREEN;
                    break;
            }
        }
    }
}

void Game::updateLevelComplete(float delta_time) {
    // Nothing to update for now
}

void Game::renderLevelComplete() {
    SDL_Color white = {255, 255, 255, 255};

    // Render "Level Complete!"
    std::string complete_text = "Level Complete!";
    SDL_Surface* complete_surface = TTF_RenderText_Blended(title_font_, complete_text.c_str(), white);
    SDL_Texture* complete_texture = SDL_CreateTextureFromSurface(renderer_, complete_surface);
    int complete_w = complete_surface->w;
    int complete_h = complete_surface->h;
    SDL_Rect complete_rect = {(SCREEN_WIDTH - complete_w) / 2, SCREEN_HEIGHT / 2 - 50, complete_w, complete_h};
    SDL_RenderCopy(renderer_, complete_texture, nullptr, &complete_rect);
    SDL_FreeSurface(complete_surface);
    SDL_DestroyTexture(complete_texture);

    // Render Prompt
    std::string prompt_text = "Press Enter to Continue";
    SDL_Surface* prompt_surface = TTF_RenderText_Blended(font_, prompt_text.c_str(), white);
    SDL_Texture* prompt_texture = SDL_CreateTextureFromSurface(renderer_, prompt_surface);
    int prompt_w = prompt_surface->w;
    int prompt_h = prompt_surface->h;
    SDL_Rect prompt_rect = {(SCREEN_WIDTH - prompt_w) / 2, SCREEN_HEIGHT - 100, prompt_w, prompt_h};
    SDL_RenderCopy(renderer_, prompt_texture, nullptr, &prompt_rect);
    SDL_FreeSurface(prompt_surface);
    SDL_DestroyTexture(prompt_texture);
}

void Game::renderLevelIntro() {
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color grey = {200, 200, 200, 255};

    if (current_level_) {
        if (const LevelData* level_data = current_level_->getCurrentLevelData()) {
        
        // Render Level Name
        std::string name_text = "Level: " + level_data->name;
        SDL_Surface* name_surface = TTF_RenderText_Blended(title_font_, name_text.c_str(), white);
        SDL_Texture* name_texture = SDL_CreateTextureFromSurface(renderer_, name_surface);
        int name_w = name_surface->w;
        int name_h = name_surface->h;
        SDL_Rect name_rect = {(SCREEN_WIDTH - name_w) / 2, 100, name_w, name_h};
        SDL_RenderCopy(renderer_, name_texture, nullptr, &name_rect);
        SDL_FreeSurface(name_surface);
        SDL_DestroyTexture(name_texture);

        // Render Description
        std::string desc_text = "Description: " + level_data->description;
        SDL_Surface* desc_surface = TTF_RenderText_Blended_Wrapped(font_, desc_text.c_str(), grey, SCREEN_WIDTH - 100);
        SDL_Texture* desc_texture = SDL_CreateTextureFromSurface(renderer_, desc_surface);
        int desc_w = desc_surface->w;
        int desc_h = desc_surface->h;
        SDL_Rect desc_rect = {(SCREEN_WIDTH - desc_w) / 2, 250, desc_w, desc_h};
        SDL_RenderCopy(renderer_, desc_texture, nullptr, &desc_rect);
        SDL_FreeSurface(desc_surface);
        SDL_DestroyTexture(desc_texture);

        // Render Difficulty
        std::string diff_text = "Difficulty: " + level_data->difficulty;
        SDL_Surface* diff_surface = TTF_RenderText_Blended(font_, diff_text.c_str(), grey);
        SDL_Texture* diff_texture = SDL_CreateTextureFromSurface(renderer_, diff_surface);
        int diff_w = diff_surface->w;
        int diff_h = diff_surface->h;
        SDL_Rect diff_rect = {(SCREEN_WIDTH - diff_w) / 2, 400, diff_w, diff_h};
        SDL_RenderCopy(renderer_, diff_texture, nullptr, &diff_rect);
        SDL_FreeSurface(diff_surface);
        SDL_DestroyTexture(diff_texture);
        }
    }

    // Render Prompt
    std::string prompt_text = "Press Enter to Start";
    SDL_Surface* prompt_surface = TTF_RenderText_Blended(font_, prompt_text.c_str(), white);
    SDL_Texture* prompt_texture = SDL_CreateTextureFromSurface(renderer_, prompt_surface);
    int prompt_w = prompt_surface->w;
    int prompt_h = prompt_surface->h;
    SDL_Rect prompt_rect = {(SCREEN_WIDTH - prompt_w) / 2, SCREEN_HEIGHT - 100, prompt_w, prompt_h};
    SDL_RenderCopy(renderer_, prompt_texture, nullptr, &prompt_rect);
    SDL_FreeSurface(prompt_surface);
    SDL_DestroyTexture(prompt_texture);
}

void Game::renderStartScreen() {
    // Set background color
    SDL_SetRenderDrawColor(renderer_, 20, 20, 50, 255); // Dark blue background
    SDL_RenderClear(renderer_);
    
    // Render title
    SDL_Color white = {255, 255, 255, 255};
    SDL_Surface* title_surface = TTF_RenderText_Blended(title_font_, "Ball Maze Game", white);
    if (title_surface) {
        SDL_Texture* title_texture = SDL_CreateTextureFromSurface(renderer_, title_surface);
        if (title_texture) {
            SDL_Rect title_rect = {SCREEN_WIDTH / 2 - title_surface->w / 2, 50, title_surface->w, title_surface->h};
            SDL_RenderCopy(renderer_, title_texture, nullptr, &title_rect);
            SDL_DestroyTexture(title_texture);
        }
        SDL_FreeSurface(title_surface);
    }
    
    // Render instructions
    SDL_Surface* instructions_surface = TTF_RenderText_Blended(font_, "Select a level pack with LEFT/RIGHT arrows. Press ENTER to start.", white);
    if (instructions_surface) {
        SDL_Texture* instructions_texture = SDL_CreateTextureFromSurface(renderer_, instructions_surface);
        if (instructions_texture) {
            SDL_Rect instructions_rect = {SCREEN_WIDTH / 2 - instructions_surface->w / 2, SCREEN_HEIGHT - 100, instructions_surface->w, instructions_surface->h};
            SDL_RenderCopy(renderer_, instructions_texture, nullptr, &instructions_rect);
            SDL_DestroyTexture(instructions_texture);
        }
        SDL_FreeSurface(instructions_surface);
    }
    
    // If no level packs found
    if (level_packs_.empty()) {
        SDL_Surface* no_levels_surface = TTF_RenderText_Blended(font_, "No level packs found in assets/levels directory", {255, 100, 100, 255});
        if (no_levels_surface) {
            SDL_Texture* no_levels_texture = SDL_CreateTextureFromSurface(renderer_, no_levels_surface);
            if (no_levels_texture) {
                SDL_Rect no_levels_rect = {SCREEN_WIDTH / 2 - no_levels_surface->w / 2, SCREEN_HEIGHT / 2, no_levels_surface->w, no_levels_surface->h};
                SDL_RenderCopy(renderer_, no_levels_texture, nullptr, &no_levels_rect);
                SDL_DestroyTexture(no_levels_texture);
            }
            SDL_FreeSurface(no_levels_surface);
        }
    } else {
        // Render level pack info
        const auto& pack = level_packs_[current_level_pack_index_];
        
        // Render level pack name
        SDL_Surface* name_surface = TTF_RenderText_Blended(title_font_, pack.name.c_str(), {255, 255, 150, 255});
        if (name_surface) {
            SDL_Texture* name_texture = SDL_CreateTextureFromSurface(renderer_, name_surface);
            if (name_texture) {
                SDL_Rect name_rect = {SCREEN_WIDTH / 2 - name_surface->w / 2, SCREEN_HEIGHT / 2 - 100, name_surface->w, name_surface->h};
                SDL_RenderCopy(renderer_, name_texture, nullptr, &name_rect);
                SDL_DestroyTexture(name_texture);
            }
            SDL_FreeSurface(name_surface);
        }
        
        // Render level pack description
        SDL_Surface* desc_surface = TTF_RenderText_Blended(font_, pack.description.c_str(), white);
        if (desc_surface) {
            SDL_Texture* desc_texture = SDL_CreateTextureFromSurface(renderer_, desc_surface);
            if (desc_texture) {
                SDL_Rect desc_rect = {SCREEN_WIDTH / 2 - desc_surface->w / 2, SCREEN_HEIGHT / 2 - 40, desc_surface->w, desc_surface->h};
                SDL_RenderCopy(renderer_, desc_texture, nullptr, &desc_rect);
                SDL_DestroyTexture(desc_texture);
            }
            SDL_FreeSurface(desc_surface);
        }
        
        // Render author
        std::string author_text = "By: " + pack.author;
        SDL_Surface* author_surface = TTF_RenderText_Blended(font_, author_text.c_str(), {200, 200, 200, 255});
        if (author_surface) {
            SDL_Texture* author_texture = SDL_CreateTextureFromSurface(renderer_, author_surface);
            if (author_texture) {
                SDL_Rect author_rect = {SCREEN_WIDTH / 2 - author_surface->w / 2, SCREEN_HEIGHT / 2 + 20, author_surface->w, author_surface->h};
                SDL_RenderCopy(renderer_, author_texture, nullptr, &author_rect);
                SDL_DestroyTexture(author_texture);
            }
            SDL_FreeSurface(author_surface);
        }
    }
}

void Game::renderGameplay() {
    updateCameraOffsets();

    // Render the maze
    if (maze_) {
        maze_->render(renderer_, camera_offset_x_, camera_offset_y_);
    }

    // Render the ball
    if (ball_) {
        ball_->render(renderer_, camera_offset_x_, camera_offset_y_);
    }

    // Render the goal
    if (current_level_ && maze_) {
        if (const LevelData* level_data = current_level_->getCurrentLevelData()) {
            b2Vec2 goal_pos_grid = level_data->goal_position;
            float maze_rotation = maze_->getCurrentRotationRad();
            b2Vec2 maze_pivot_point = maze_->getMazeCenterWorldCoords();

            b2Vec2 initial_goal_pos_meters = {
                maze_world_origin_meters_.x + (goal_pos_grid.x + 0.5f) * (TILE_SIZE / PPM),
                maze_world_origin_meters_.y + (goal_pos_grid.y + 0.5f) * (TILE_SIZE / PPM)
            };

            b2Vec2 offset_from_pivot = initial_goal_pos_meters - maze_pivot_point;
            float rotated_offset_x = offset_from_pivot.x * cosf(maze_rotation) - offset_from_pivot.y * sinf(maze_rotation);
            float rotated_offset_y = offset_from_pivot.x * sinf(maze_rotation) + offset_from_pivot.y * cosf(maze_rotation);
            b2Vec2 current_goal_pos_meters = maze_pivot_point + b2Vec2{rotated_offset_x, rotated_offset_y};

            int screen_x = static_cast<int>(current_goal_pos_meters.x * PPM + camera_offset_x_);
            int screen_y = static_cast<int>(current_goal_pos_meters.y * PPM + camera_offset_y_);
            int tile_size_pixels = TILE_SIZE;

            int radius = tile_size_pixels / 2;
            SDL_SetRenderDrawColor(renderer_, 0, 255, 0, 255); // Green for the goal

            // Draw a filled circle by drawing horizontal lines
            for (int dy = -radius; dy <= radius; ++dy) {
                int dx = static_cast<int>(sqrt((radius * radius) - (dy * dy)));
                SDL_RenderDrawLine(renderer_, screen_x - dx, screen_y + dy, screen_x + dx, screen_y + dy);
            }
        }
    }

    // Render warps
    for (const auto& warp : warps_) {
        if (warp) {
            warp->render(renderer_, camera_offset_x_, camera_offset_y_);
        }
    }

    // Render reverse items
    if (maze_) { // Ensure maze_ is valid for rotation calculations
        float maze_rotation_render = maze_->getCurrentRotationRad();
        b2Vec2 maze_pivot_point_render = maze_->getMazeCenterWorldCoords();

        for (const auto& reverse_item : reverse_items_) {
            if (reverse_item && reverse_item->isActive()) { // Render if active, cooldown state is handled by item's render method
                b2Vec2 item_original_pos = reverse_item->getPosition();
                
                // Calculate the rotated position for rendering
                b2Vec2 offset_from_pivot_render = item_original_pos - maze_pivot_point_render;
                float rotated_offset_x_render = offset_from_pivot_render.x * cosf(maze_rotation_render) - offset_from_pivot_render.y * sinf(maze_rotation_render);
                float rotated_offset_y_render = offset_from_pivot_render.x * sinf(maze_rotation_render) + offset_from_pivot_render.y * cosf(maze_rotation_render);
                b2Vec2 rotated_item_render_pos = maze_pivot_point_render + b2Vec2{rotated_offset_x_render, rotated_offset_y_render};
                
                reverse_item->render(renderer_, camera_offset_x_, camera_offset_y_, rotated_item_render_pos);
            }
        }
    } else { // Fallback if maze_ is null, render items at their original positions
        for (const auto& reverse_item : reverse_items_) {
            if (reverse_item && reverse_item->isActive()) {
                reverse_item->render(renderer_, camera_offset_x_, camera_offset_y_); // Pass default rotated_position
            }
        }
    }

    // Render holes
    if (current_level_ && maze_) {
        const auto& hole_grid_positions = current_level_->getHolePositions();
        float hole_radius_pixels = (TILE_SIZE / 2.0f) * 0.9f; // 90% of half tile size

        float maze_rotation = maze_->getCurrentRotationRad();
        b2Vec2 maze_pivot_point = maze_->getMazeCenterWorldCoords();

        SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255); // Black color for holes

        for (const auto& grid_pos : hole_grid_positions) {
            b2Vec2 initial_hole_world_pos = {
                maze_world_origin_meters_.x + (grid_pos.x + 0.5f) * (TILE_SIZE / PPM),
                maze_world_origin_meters_.y + (grid_pos.y + 0.5f) * (TILE_SIZE / PPM)
            };

            b2Vec2 offset_from_pivot = initial_hole_world_pos - maze_pivot_point;
            float rotated_offset_x = offset_from_pivot.x * cosf(maze_rotation) - offset_from_pivot.y * sinf(maze_rotation);
            float rotated_offset_y = offset_from_pivot.x * sinf(maze_rotation) + offset_from_pivot.y * cosf(maze_rotation);
            b2Vec2 current_hole_pos_meters = maze_pivot_point + b2Vec2{rotated_offset_x, rotated_offset_y};

            int screen_x = static_cast<int>(current_hole_pos_meters.x * PPM + camera_offset_x_);
            int screen_y = static_cast<int>(current_hole_pos_meters.y * PPM + camera_offset_y_);

            // Draw a filled circle by drawing horizontal lines
            for (int dy = -hole_radius_pixels; dy <= hole_radius_pixels; ++dy) {
                int dx = static_cast<int>(sqrt((hole_radius_pixels * hole_radius_pixels) - (dy * dy)));
                SDL_RenderDrawLine(renderer_, screen_x - dx, screen_y + dy, screen_x + dx, screen_y + dy);
            }
        }
    }

    // Render status messages (e.g., warp cooldown, reverse effect duration)
    SDL_Color white = {255, 255, 255, 255};
    std::string status_text;
    if (is_warp_cooldown_) {
        status_text += "Warp Cooldown! ";
    }
    // Removed Controls Inverted text
    // if (controls_inverted_) {
    //     status_text += "Controls Inverted! (";
    //     status_text += std::to_string(static_cast<int>(std::ceil(reverse_effect_timer_)));
    //     status_text += "s) ";
    // }

    if (!status_text.empty()) {
        SDL_Surface* surface = TTF_RenderText_Blended(font_, status_text.c_str(), white);
        if (surface) {
            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer_, surface);
            if (texture) {
                SDL_Rect dstrect = {10, 10, surface->w, surface->h};
                SDL_RenderCopy(renderer_, texture, nullptr, &dstrect);
                SDL_DestroyTexture(texture);
            }
            SDL_FreeSurface(surface);
        }
    }
} // Correct closing brace for renderGameplay

void Game::createMazeAndBall() {
    // Guard clause: ensure a level is loaded.
    if (!current_level_ || !current_level_->getCurrentLevelData()) {
        std::cerr << "Cannot create maze and ball: No level data loaded" << std::endl;
        return;
    }

    // 1. Complete Cleanup of the previous level
    if (maze_) maze_.reset();
    if (ball_) ball_.reset();
    reverse_items_.clear();
    warps_.clear();

    // 2. Reset all level-specific states and timers
    is_level_won_ = false;
    controls_inverted_ = false;
    reverse_effect_timer_ = 0.0f;
    is_warp_cooldown_ = false;
    warp_cooldown_timer_ = 0.0f;

    // 3. Re-creation of the new level
    const LevelData* levelData = current_level_->getCurrentLevelData();

    // Center the maze on the screen
    float maze_width_meters = levelData->width * (TILE_SIZE / PPM);
    float maze_height_meters = levelData->height * (TILE_SIZE / PPM);
    float screen_width_meters = SCREEN_WIDTH / PPM;
    float screen_height_meters = SCREEN_HEIGHT / PPM;
    maze_world_origin_meters_ = {
        (screen_width_meters - maze_width_meters) / 2.0f,
        (screen_height_meters - maze_height_meters) / 2.0f
    };

    // Create the maze
    maze_ = std::make_unique<Maze>(worldId_);
    maze_->create(*current_level_, maze_world_origin_meters_);

    // Create the ball
    b2Vec2 ball_start_grid_pos = levelData->ball_start_position;
    float ball_start_x_meters = maze_world_origin_meters_.x + (ball_start_grid_pos.x + 0.5f) * (TILE_SIZE / PPM);
    float ball_start_y_meters = maze_world_origin_meters_.y + (ball_start_grid_pos.y + 0.5f) * (TILE_SIZE / PPM);
    float ball_radius_meters = (TILE_SIZE / PPM) * 0.45f;
    ball_ = std::make_unique<Ball>(worldId_);
    ball_->create({ball_start_x_meters, ball_start_y_meters}, ball_radius_meters);

    // Create warp objects
    for (const auto& [id, pos] : levelData->warp_positions) {
        b2Vec2 world_pos = {
            maze_world_origin_meters_.x + (pos.x + 0.5f) * (TILE_SIZE / PPM),
            maze_world_origin_meters_.y + (pos.y + 0.5f) * (TILE_SIZE / PPM)
        };
        warps_.push_back(std::make_unique<Warp>(worldId_, id, world_pos, (TILE_SIZE / PPM) * 0.5f));
    }

    // Create reverse items
    for (const auto& pos : levelData->reverse_item_positions) {
        float item_x_meters = maze_world_origin_meters_.x + (pos.x + 0.5f) * (TILE_SIZE / PPM);
        float item_y_meters = maze_world_origin_meters_.y + (pos.y + 0.5f) * (TILE_SIZE / PPM);
        float item_size_meters = (TILE_SIZE / PPM) * 0.8f;
        auto reverse_item = std::make_unique<ReverseItem>(worldId_);
        reverse_item->create({item_x_meters, item_y_meters}, item_size_meters);
        reverse_items_.push_back(std::move(reverse_item));
    }

    // 4. Update the camera
    updateCameraOffsets();

    std::cout << "Successfully created level: " << levelData->name 
              << " (#" << current_level_->getCurrentLevelIndex() + 1 << ")" << std::endl;
}

void Game::updateCameraOffsets() {
    if (!current_level_ || !maze_) {
        camera_offset_x_ = 0.0f;
        camera_offset_y_ = 0.0f;
        return;
    }

    const float TILE_SIZE_M = TILE_SIZE / PPM;
    float unrotated_maze_width_m = current_level_->getWidth() * TILE_SIZE_M;
    float unrotated_maze_height_m = current_level_->getHeight() * TILE_SIZE_M;

    b2Vec2 unrotated_abs_corners_m[4];
    unrotated_abs_corners_m[0] = maze_world_origin_meters_;
    unrotated_abs_corners_m[1] = {maze_world_origin_meters_.x + unrotated_maze_width_m, maze_world_origin_meters_.y};
    unrotated_abs_corners_m[2] = {maze_world_origin_meters_.x, maze_world_origin_meters_.y + unrotated_maze_height_m};
    unrotated_abs_corners_m[3] = {maze_world_origin_meters_.x + unrotated_maze_width_m, maze_world_origin_meters_.y + unrotated_maze_height_m};

    b2Vec2 rotation_pivot_abs_m = {
        maze_world_origin_meters_.x + unrotated_maze_width_m / 2.0f,
        maze_world_origin_meters_.y + unrotated_maze_height_m / 2.0f
    };

    float angle_rad = maze_->getCurrentRotationRad();
    float cos_a = std::cos(angle_rad);
    float sin_a = std::sin(angle_rad);

    float min_rotated_x_m = std::numeric_limits<float>::max();
    float max_rotated_x_m = std::numeric_limits<float>::lowest();
    float min_rotated_y_m = std::numeric_limits<float>::max();
    float max_rotated_y_m = std::numeric_limits<float>::lowest();

    for (int i = 0; i < 4; ++i) {
        float translated_x = unrotated_abs_corners_m[i].x - rotation_pivot_abs_m.x;
        float translated_y = unrotated_abs_corners_m[i].y - rotation_pivot_abs_m.y;
        float rotated_translated_x = translated_x * cos_a - translated_y * sin_a;
        float rotated_translated_y = translated_x * sin_a + translated_y * cos_a;
        float final_x_m = rotated_translated_x + rotation_pivot_abs_m.x;
        float final_y_m = rotated_translated_y + rotation_pivot_abs_m.y;

        min_rotated_x_m = std::min(min_rotated_x_m, final_x_m);
        max_rotated_x_m = std::max(max_rotated_x_m, final_x_m);
        min_rotated_y_m = std::min(min_rotated_y_m, final_y_m);
        max_rotated_y_m = std::max(max_rotated_y_m, final_y_m);
    }

    float effective_maze_origin_x_pixels = min_rotated_x_m * PPM;
    float effective_maze_origin_y_pixels = min_rotated_y_m * PPM;
    float effective_maze_width_pixels = (max_rotated_x_m - min_rotated_x_m) * PPM;
    float effective_maze_height_pixels = (max_rotated_y_m - min_rotated_y_m) * PPM;

    if (effective_maze_width_pixels <= SCREEN_WIDTH && effective_maze_height_pixels <= SCREEN_HEIGHT) {
        camera_offset_x_ = (SCREEN_WIDTH - effective_maze_width_pixels) / 2.0f - effective_maze_origin_x_pixels;
        camera_offset_y_ = (SCREEN_HEIGHT - effective_maze_height_pixels) / 2.0f - effective_maze_origin_y_pixels;
    } else if (ball_) {
        b2Vec2 ball_pos_m = ball_->getPosition();
        float ball_screen_x_abs_pixels = ball_pos_m.x * PPM;
        float ball_screen_y_abs_pixels = ball_pos_m.y * PPM;

        float desired_camera_offset_x = SCREEN_WIDTH / 2.0f - ball_screen_x_abs_pixels;
        float desired_camera_offset_y = SCREEN_HEIGHT / 2.0f - ball_screen_y_abs_pixels;

        float min_bound_camera_x = SCREEN_WIDTH - (effective_maze_origin_x_pixels + effective_maze_width_pixels);
        float max_bound_camera_x = -effective_maze_origin_x_pixels;
        float min_bound_camera_y = SCREEN_HEIGHT - (effective_maze_origin_y_pixels + effective_maze_height_pixels);
        float max_bound_camera_y = -effective_maze_origin_y_pixels;

        float target_camera_offset_x = std::max(min_bound_camera_x, std::min(max_bound_camera_x, desired_camera_offset_x));
        float target_camera_offset_y = std::max(min_bound_camera_y, std::min(max_bound_camera_y, desired_camera_offset_y));
        
        const float CAMERA_SMOOTHING_FACTOR = 0.3f;
        camera_offset_x_ += (target_camera_offset_x - camera_offset_x_) * CAMERA_SMOOTHING_FACTOR;
        camera_offset_y_ += (target_camera_offset_y - camera_offset_y_) * CAMERA_SMOOTHING_FACTOR;
    } else {
        camera_offset_x_ = (SCREEN_WIDTH - effective_maze_width_pixels) / 2.0f - effective_maze_origin_x_pixels;
        camera_offset_y_ = (SCREEN_HEIGHT - effective_maze_height_pixels) / 2.0f - effective_maze_origin_y_pixels;
        camera_offset_x_ = std::max(SCREEN_WIDTH - effective_maze_width_pixels - effective_maze_origin_x_pixels, std::min(-effective_maze_origin_x_pixels, camera_offset_x_));
        camera_offset_y_ = std::max(SCREEN_HEIGHT - effective_maze_height_pixels - effective_maze_origin_y_pixels, std::min(-effective_maze_origin_y_pixels, camera_offset_y_));
    }
}

void Game::cleanup() {
    if (ball_) {
        ball_.reset();
    }
    if (maze_) {
        maze_.reset();
    }
    
    // Clean up reverse items
    for (auto& item : reverse_items_) {
        item.reset();
    }
    reverse_items_.clear();
    
    // Clean up warp items
    for (auto& warp : warps_) {
        warp.reset();
    }
    warps_.clear();
    
    // Clean up Box2D world
    if (b2World_IsValid(worldId_)) {
        b2DestroyWorld(worldId_);
        worldId_ = b2_nullWorldId;
    }
    
    // Clean up fonts
    if (font_) {
        TTF_CloseFont(font_);
        font_ = nullptr;
    }
    
    if (title_font_) {
        TTF_CloseFont(title_font_);
        title_font_ = nullptr;
    }
    
    // Clean up SDL
    if (renderer_) {
        SDL_DestroyRenderer(renderer_);
        renderer_ = nullptr;
    }
    
    if (window_) {
        SDL_DestroyWindow(window_);
        window_ = nullptr;
    }
    
    // Quit SDL_ttf
    TTF_Quit();
    
    // Quit SDL
    SDL_Quit();
}

// Helper function to draw a circle using SDL_RenderDrawPoint
void SDL_RenderDrawCircle(SDL_Renderer* renderer, int center_x, int center_y, int radius) {
    const int diameter = radius * 2;
    int x = radius - 1;
    int y = 0;
    int tx = 1;
    int ty = 1;
    int error = tx - diameter;

    while (x >= y) {
        // Draw 8 points for each octant
        SDL_RenderDrawPoint(renderer, center_x + x, center_y - y);
        SDL_RenderDrawPoint(renderer, center_x + x, center_y + y);
        SDL_RenderDrawPoint(renderer, center_x - x, center_y - y);
        SDL_RenderDrawPoint(renderer, center_x - x, center_y + y);
        SDL_RenderDrawPoint(renderer, center_x + y, center_y - x);
        SDL_RenderDrawPoint(renderer, center_x + y, center_y + x);
        SDL_RenderDrawPoint(renderer, center_x - y, center_y - x);
        SDL_RenderDrawPoint(renderer, center_x - y, center_y + x);

        if (error <= 0) {
            y++;
            error += ty;
            ty += 2;
        }
        if (error > 0) {
            x--;
            tx += 2;
            error += tx - diameter;
        }
    }
}

// renderBox2DBody can be implemented later if needed for debugging complex shapes.
// For now, Ball and Maze handle their own specific rendering.
void Game::renderBox2DBody(SDL_Renderer* renderer, b2BodyId bodyId, SDL_Color color, float camera_offset_x, float camera_offset_y) {
    // Implementation can be added later if needed
}
