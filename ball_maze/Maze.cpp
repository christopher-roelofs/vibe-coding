#include "Maze.h"
#include <cmath> // For M_PI, cos, sin
#include <iostream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Maze::Maze(b2WorldId worldId) : 
    worldId_(worldId), 
    maze_body_id_(b2_nullBodyId),
    current_rotation_rad_(0.0f), 
    target_rotation_rad_(0.0f), 
    rotation_direction_(0),
    tile_size_meters_(0.0f)
{
    maze_center_world_coords_ = {0.0f, 0.0f};
}

Maze::~Maze() {
    if (b2Body_IsValid(maze_body_id_)) { // Destroy the single maze body
        b2DestroyBody(maze_body_id_);
        maze_body_id_ = b2_nullBodyId;
    }
    wall_segments_.clear(); // Clear the visual segment data
}

void Maze::create(const Level& level, b2Vec2 maze_world_origin_meters) {
    // --- Cleanup and Reset ---
    if (b2Body_IsValid(maze_body_id_)) {
        b2DestroyBody(maze_body_id_);
        maze_body_id_ = b2_nullBodyId;
    }
    wall_segments_.clear();

    // Explicitly reset all rotation variables to their default states
    current_rotation_rad_ = 0.0f;
    target_rotation_rad_ = 0.0f;
    rotation_direction_ = 0;

    tile_size_meters_ = TILE_SIZE / PPM;
    const auto& layout = level.getLayout();
    int grid_height = level.getHeight();
    int grid_width = level.getWidth();

    float maze_pixel_width_meters = grid_width * tile_size_meters_;
    float maze_pixel_height_meters = grid_height * tile_size_meters_;
    this->maze_center_world_coords_ = {
        maze_world_origin_meters.x + maze_pixel_width_meters / 2.0f,
        maze_world_origin_meters.y + maze_pixel_height_meters / 2.0f
    };

    // Create the single kinematic body for the entire maze
    // Changed from static to kinematic to enable continuous rotation via angular velocity
    b2BodyDef maze_body_def = b2DefaultBodyDef();
    maze_body_def.type = b2_kinematicBody;
    maze_body_def.position = this->maze_center_world_coords_; // Position the body at its pivot
    maze_body_id_ = b2CreateBody(worldId_, &maze_body_def);

    if (!b2Body_IsValid(maze_body_id_)) {
        std::cerr << "Error: Failed to create the main maze body!" << std::endl;
        return;
    }

    for (int r = 0; r < grid_height; ++r) {
        for (int c = 0; c < grid_width; ++c) {
            if (layout[r][c] == '#') {
                // Calculate the center of this wall segment in world coordinates
                b2Vec2 initial_wall_center_world_meters = {
                    maze_world_origin_meters.x + (c + 0.5f) * tile_size_meters_,
                    maze_world_origin_meters.y + (r + 0.5f) * tile_size_meters_
                };
                
                // Calculate the offset of this wall segment from the maze's central pivot point
                b2Vec2 offset_from_pivot = initial_wall_center_world_meters - this->maze_center_world_coords_;

                // Create the shape (box) for this wall segment
                // Add overlap to physics shape dimensions
                float physics_hx = (tile_size_meters_ / 2.0f) + PHYSICS_SHAPE_OVERLAP_METERS;
                float physics_hy = (tile_size_meters_ / 2.0f) + PHYSICS_SHAPE_OVERLAP_METERS;

                // 1. Create a base polygon centered at (0,0)
                b2Polygon wall_polygon = b2MakeBox(physics_hx, physics_hy);

                // 2. Define the local transform for this segment
                b2Transform local_transform = b2Transform_identity; // Correct way to get identity transform
                local_transform.p = offset_from_pivot; // Apply the translation
                // local_transform.q remains identity (no local rotation of the shape itself)

                // 3. Apply the local transform to the polygon's vertices and get the new polygon
                wall_polygon = b2TransformPolygon(local_transform, &wall_polygon); // Correct usage
                
                b2ShapeDef wall_fixture_def = b2DefaultShapeDef();
                wall_fixture_def.density = 0.0f; // Static bodies have 0 density/mass
                wall_fixture_def.material.friction = 0.5f;
                wall_fixture_def.material.restitution = 0.0f;  // No bounciness
                // Add this shape to the single maze body
                b2CreatePolygonShape(maze_body_id_, &wall_fixture_def, &wall_polygon);

                WallSegment segment; // Use WallSegment as per Maze.h
                segment.original_offset_from_center_meters = offset_from_pivot;
                segment.size_meters = {tile_size_meters_, tile_size_meters_};
                wall_segments_.push_back(segment);
            }
        }
    }
    // No need to call applyCurrentRotationToBodies() here, 
    // the body is created with 0 rotation at its center.

    // Add an invisible outer boundary around the entire maze as a safety net
    // This will prevent the ball from escaping if it somehow clips through a wall
    float boundary_margin = 0.5f; // Extra margin beyond the maze edges
    float boundary_width = maze_pixel_width_meters + 2.0f * boundary_margin;
    float boundary_height = maze_pixel_height_meters + 2.0f * boundary_margin;
    
    // Create the four boundary walls (top, right, bottom, left)
    float wall_thickness = 0.2f; // Thickness of the boundary walls
    
    // Top boundary
    b2Polygon top_wall = b2MakeBox(boundary_width / 2.0f, wall_thickness / 2.0f);
    b2Transform top_transform = b2Transform_identity;
    top_transform.p = {0.0f, -boundary_height / 2.0f};
    top_wall = b2TransformPolygon(top_transform, &top_wall);
    b2ShapeDef top_wall_def = b2DefaultShapeDef();
    top_wall_def.density = 0.0f;
    top_wall_def.material.friction = 0.5f;
    b2CreatePolygonShape(maze_body_id_, &top_wall_def, &top_wall);
    
    // Bottom boundary
    b2Polygon bottom_wall = b2MakeBox(boundary_width / 2.0f, wall_thickness / 2.0f);
    b2Transform bottom_transform = b2Transform_identity;
    bottom_transform.p = {0.0f, boundary_height / 2.0f};
    bottom_wall = b2TransformPolygon(bottom_transform, &bottom_wall);
    b2ShapeDef bottom_wall_def = b2DefaultShapeDef();
    bottom_wall_def.density = 0.0f;
    bottom_wall_def.material.friction = 0.5f;
    b2CreatePolygonShape(maze_body_id_, &bottom_wall_def, &bottom_wall);
    
    // Left boundary
    b2Polygon left_wall = b2MakeBox(wall_thickness / 2.0f, boundary_height / 2.0f);
    b2Transform left_transform = b2Transform_identity;
    left_transform.p = {-boundary_width / 2.0f, 0.0f};
    left_wall = b2TransformPolygon(left_transform, &left_wall);
    b2ShapeDef left_wall_def = b2DefaultShapeDef();
    left_wall_def.density = 0.0f;
    left_wall_def.material.friction = 0.5f;
    b2CreatePolygonShape(maze_body_id_, &left_wall_def, &left_wall);
    
    // Right boundary
    b2Polygon right_wall = b2MakeBox(wall_thickness / 2.0f, boundary_height / 2.0f);
    b2Transform right_transform = b2Transform_identity;
    right_transform.p = {boundary_width / 2.0f, 0.0f};
    right_wall = b2TransformPolygon(right_transform, &right_wall);
    b2ShapeDef right_wall_def = b2DefaultShapeDef();
    right_wall_def.density = 0.0f;
    right_wall_def.material.friction = 0.5f;
    b2CreatePolygonShape(maze_body_id_, &right_wall_def, &right_wall);
}

void Maze::render(SDL_Renderer* renderer, float camera_offset_x, float camera_offset_y) const {
    if (!b2Body_IsValid(maze_body_id_)) {
        return; // Nothing to render if the main maze body isn't valid
    }

    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255); // Grey walls

    // Get the current transform of the entire maze body
    b2Transform maze_body_transform = b2Body_GetTransform(maze_body_id_);

    for (const auto& segment : wall_segments_) {
        // Half-dimensions of the current wall segment
        float hx = segment.size_meters.x / 2.0f;
        float hy = segment.size_meters.y / 2.0f;

        // Define the 4 corners of this wall segment in its own local coordinate system
        // (i.e., relative to its own center, which is segment.original_offset_from_center_meters
        //  when expressed in the maze body's frame).
        b2Vec2 segment_local_corners[4] = {
            {-hx, -hy}, // Top-left
            { hx, -hy}, // Top-right
            { hx,  hy}, // Bottom-right
            {-hx,  hy}  // Bottom-left
        };

        SDL_Point screen_points[5]; // 4 corners + 1 to close the polygon

        for (int i = 0; i < 4; ++i) {
            // 1. Get the segment's corner position relative to the segment's center.
            b2Vec2 corner_relative_to_segment_center = segment_local_corners[i];

            // 2. Add the segment's center offset (from maze body origin) to get the corner's position
            //    in the maze body's local coordinate system.
            b2Vec2 corner_in_maze_body_frame = segment.original_offset_from_center_meters + corner_relative_to_segment_center;
            
            // 3. Transform this point from the maze body's local frame to world coordinates.
            b2Vec2 world_corner_position = b2TransformPoint(maze_body_transform, corner_in_maze_body_frame);

            // 4. Convert world coordinates to screen coordinates
            screen_points[i].x = static_cast<int>((world_corner_position.x * PPM) + camera_offset_x);
            screen_points[i].y = static_cast<int>((world_corner_position.y * PPM) + camera_offset_y);
        }
        screen_points[4] = screen_points[0]; // Close the polygon by repeating the first point

        SDL_RenderDrawLines(renderer, screen_points, 5);
    }
}

void Maze::applyCurrentRotationToBodies() {
    if (b2Body_IsValid(maze_body_id_)) {
        b2Vec2 current_body_pos = b2Body_GetPosition(maze_body_id_);
        if (fabs(current_body_pos.x - maze_center_world_coords_.x) > 0.0001f || 
            fabs(current_body_pos.y - maze_center_world_coords_.y) > 0.0001f) {
            std::cout << "WARNING: Maze body position drifted! Expected (" 
                      << maze_center_world_coords_.x << "," << maze_center_world_coords_.y 
                      << "), Got (" << current_body_pos.x << "," << current_body_pos.y << ")" << std::endl;
        }

        // The body's position (maze_center_world_coords_) is its pivot and is set at creation.
        // We only need to update its rotation around this pivot.
        b2Rot new_rotation = b2MakeRot(current_rotation_rad_);
        // The position argument to b2Body_SetTransform is the new world position of the body's origin.
        // Since our maze_body_id_ is already positioned at its pivot (maze_center_world_coords_),
        // and this pivot point itself doesn't move in the world, we pass maze_center_world_coords_.
        b2Body_SetTransform(maze_body_id_, maze_center_world_coords_, new_rotation);
    }
}

// Set rotation direction and apply angular velocity directly to the maze body
void Maze::setRotationDirection(int direction) {
    rotation_direction_ = direction;
    
    if (b2Body_IsValid(maze_body_id_)) {
        // Convert rotation speed from degrees/sec to radians/sec
        float angular_velocity = static_cast<float>(direction) * (MAZE_TARGET_ROTATION_SPEED_DPS * M_PI / 180.0f);
        
        // Apply angular velocity directly to the Box2D body
        b2Body_SetAngularVelocity(maze_body_id_, angular_velocity);
        
        // When stopping, make sure to set the target to current rotation
        if (direction == 0) {
            target_rotation_rad_ = current_rotation_rad_;
        }
    }
}

// Update method for continuous rotation using Box2D's angular velocity
void Maze::update(float delta_time) {
    if (!b2Body_IsValid(maze_body_id_)) {
        return;
    }
    
    // Update current_rotation_rad_ from the actual Box2D body rotation
    b2Transform transform = b2Body_GetTransform(maze_body_id_);
    b2Rot rotation = transform.q;
    current_rotation_rad_ = b2Rot_GetAngle(rotation);
    
    // Normalize current_rotation_rad_ to [0, 2π)
    current_rotation_rad_ = fmodf(current_rotation_rad_, 2.0f * static_cast<float>(M_PI));
    if (current_rotation_rad_ < 0) {
        current_rotation_rad_ += 2.0f * static_cast<float>(M_PI);
    }
    
    // If we're not actively rotating and the body still has angular velocity,
    // we might want to dampen it to prevent unwanted drift
    if (rotation_direction_ == 0) {
        float current_angular_velocity = b2Body_GetAngularVelocity(maze_body_id_);
        if (fabs(current_angular_velocity) > 0.001f) {
            // Apply damping to gradually stop rotation
            float damped_velocity = current_angular_velocity * 0.9f;
            b2Body_SetAngularVelocity(maze_body_id_, damped_velocity);
        } else if (fabs(current_angular_velocity) <= 0.001f) {
            // If very slow, just stop completely
            b2Body_SetAngularVelocity(maze_body_id_, 0.0f);
        }
    }
    
    // Update target_rotation_rad_ based on current rotation if actively rotating
    if (rotation_direction_ != 0) {
        target_rotation_rad_ = current_rotation_rad_;
    }
}

// No longer using discrete rotation steps - continuous rotation via angular velocity is used instead

void Maze::resetRotation() {
    current_rotation_rad_ = 0.0f;
    target_rotation_rad_ = 0.0f;
    
    if (b2Body_IsValid(maze_body_id_)) {
        // Stop any rotation
        b2Body_SetAngularVelocity(maze_body_id_, 0.0f);
        // Reset position
        b2Body_SetTransform(maze_body_id_, maze_center_world_coords_, b2MakeRot(0.0f));
    }
} // Immediately update physical bodies

float Maze::getCurrentRotationRad() const {
    return current_rotation_rad_;
}

b2Vec2 Maze::getMazeCenterWorldCoords() const {
    return maze_center_world_coords_;
}

bool Maze::isRotating() const {
    // We're rotating if either we have a non-zero rotation direction command
    // or the body has non-zero angular velocity
    if (!b2Body_IsValid(maze_body_id_)) {
        return false;
    }
    
    float angular_velocity = b2Body_GetAngularVelocity(maze_body_id_);
    return rotation_direction_ != 0 || fabs(angular_velocity) > 0.001f;
}
