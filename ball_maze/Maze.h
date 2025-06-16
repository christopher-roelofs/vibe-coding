#ifndef MAZE_H
#define MAZE_H

#include <vector>
#include <SDL2/SDL.h>
#include <box2d/box2d.h>
#include "constants.h"
#include "Level.h"

struct WallSegment {
    b2Vec2 original_offset_from_center_meters; // Relative to maze center, before rotation
    b2Vec2 size_meters;
};

class Maze {
public:
    Maze(b2WorldId worldId);
    ~Maze();

    void create(const Level& level, b2Vec2 maze_world_origin_meters);
    void render(SDL_Renderer* renderer, float camera_offset_x, float camera_offset_y) const;
    void setRotationDirection(int direction); // -1 for left, 1 for right, 0 for stop
    void update(float delta_time); // Smoothly rotates towards target_rotation_rad_ using discrete steps
    void resetRotation(); // Resets maze rotation to 0

    // Getters for Game class to use for rotating other elements
    float getCurrentRotationRad() const;
    b2Vec2 getMazeCenterWorldCoords() const;
    bool isRotating() const; // Check if maze is currently rotating
    
    // No longer need discrete rotation step method - using continuous rotation

private:
    void applyCurrentRotationToBodies();

    b2WorldId worldId_;
    std::vector<WallSegment> wall_segments_; // Stores visual/geometric info for rendering
    b2BodyId maze_body_id_; // Single body for the entire maze structure
    b2Vec2 maze_center_world_coords_; // Calculated center of the maze in world space
    float current_rotation_rad_;      // Changed from degrees to radians
    float target_rotation_rad_;       // The rotation angle the maze is moving towards
    int rotation_direction_ = 0; // -1 for left, 1 for right, 0 for stop
    float tile_size_meters_;
    
    // Using continuous rotation via angular velocity now
    // SDL_Texture* wall_texture_; // Optional
};

#endif // MAZE_H
