#include "Warp.h"
#include "constants.h"
#include <iostream>

Warp::Warp(b2WorldId worldId, int id, b2Vec2 position, float radius)
    : worldId_(worldId), id_(id), position_(position), radius_(radius) {
    
    // Create a static body for the warp
    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = b2_staticBody;
    bodyDef.position = position_;
    
    bodyId_ = b2CreateBody(worldId_, &bodyDef);
    if (!b2Body_IsValid(bodyId_)) {
        std::cerr << "Error: Failed to create warp body!" << std::endl;
        return;
    }
    
    // Create a circle shape for the warp
    b2Circle circle;
    // In Box2D 3.x, the circle center is at the origin by default
    circle.radius = radius_; // Use the radius passed to the constructor
    
    // Create shape definition
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.isSensor = true; // Make it a sensor (no collision response)
    shapeDef.userData = reinterpret_cast<void*>(static_cast<uintptr_t>(id_)); // Store the ID in user data
    
    b2CreateCircleShape(bodyId_, &shapeDef, &circle);
}

// Helper function to draw a circle outline
static void DrawWarpCircle(SDL_Renderer* renderer, int center_x, int center_y, int radius)
{
    // Midpoint circle algorithm
    int x = radius - 1;
    int y = 0;
    int dx = 1;
    int dy = 1;
    int err = dx - (radius << 1);

    while (x >= y)
    {
        SDL_RenderDrawPoint(renderer, center_x + x, center_y + y);
        SDL_RenderDrawPoint(renderer, center_x + y, center_y + x);
        SDL_RenderDrawPoint(renderer, center_x - y, center_y + x);
        SDL_RenderDrawPoint(renderer, center_x - x, center_y + y);
        SDL_RenderDrawPoint(renderer, center_x - x, center_y - y);
        SDL_RenderDrawPoint(renderer, center_x - y, center_y - x);
        SDL_RenderDrawPoint(renderer, center_x + y, center_y - x);
        SDL_RenderDrawPoint(renderer, center_x + x, center_y - y);

        if (err <= 0)
        {
            y++;
            err += dy;
            dy += 2;
        }
        if (err > 0)
        {
            x--;
            dx += 2;
            err += dx - (radius << 1);
        }
    }
}

Warp::~Warp() {
    if (b2Body_IsValid(bodyId_)) {
        b2DestroyBody(bodyId_);
        bodyId_ = b2_nullBodyId;
    }
}

void Warp::updateCooldown(float delta_time) {
    if (is_on_cooldown_) {
        cooldown_timer_ -= delta_time;
        if (cooldown_timer_ <= 0.0f) {
            is_on_cooldown_ = false;
            cooldown_timer_ = 0.0f;
        }
    }
}

b2Vec2 Warp::getPosition() const {
    if (b2Body_IsValid(bodyId_)) {
        return b2Body_GetPosition(bodyId_);
    }
    return position_;
}

SDL_Color Warp::getColor() const {
    // Different colors for different warp IDs
    switch(id_ % 6) {
        case 0:  return {255, 0, 0, 255};     // Red
        case 1:  return {0, 255, 0, 255};     // Green
        case 2:  return {0, 0, 255, 255};     // Blue
        case 3:  return {255, 255, 0, 255};   // Yellow
        case 4:  return {255, 0, 255, 255};   // Magenta
        default: return {128, 0, 128, 255};   // Dark Purple
    }
}

void Warp::handleWarpCollision(int warpId, b2BodyId ballBody, const std::vector<std::unique_ptr<Warp>>& warps) {
    // Find the warp the ball is currently colliding with and the destination warp
    Warp* sourceWarp = nullptr;
    Warp* destWarp = nullptr;
    float minDistance = FLT_MAX;
    
    // Get ball position
    b2Vec2 ballPos = b2Body_GetPosition(ballBody);
    
    // First, find the source warp (the one closest to the ball)
    for (const auto& warp : warps) {
        if (warp->getId() == warpId) {
            // Skip warps that are on cooldown
            if (warp->isOnCooldown()) {
                continue;
            }
            
            b2Vec2 warpPos = warp->getPosition();
            b2Vec2 delta = b2Sub(ballPos, warpPos);
            float distSquared = b2Dot(delta, delta);
            
            if (distSquared < minDistance) {
                minDistance = distSquared;
                sourceWarp = warp.get();
            }
        }
    }
    
    // If no valid source warp found (all might be on cooldown), return
    if (!sourceWarp) {
        return;
    }
    
    // Now find the destination warp (any other warp with the same ID)
    for (const auto& warp : warps) {
        if (warp->getId() == warpId && warp.get() != sourceWarp) {
            destWarp = warp.get();
            break;
        }
    }
    
    // If we couldn't find a pair of warps, log and return
    if (!sourceWarp || !destWarp) {
        std::cout << "Warning: Could not find a valid warp pair with ID " << warpId << std::endl;
        return;
    }
    
    // If we found both warps, teleport the ball
    if (sourceWarp && destWarp) {
        // Get current velocity to maintain it after warp
        b2Vec2 velocity = b2Body_GetLinearVelocity(ballBody);
        
        // Calculate offset from this warp's center
        // b2Vec2 ballPos = b2Body_GetPosition(ballBody); // Unused for now
        // b2Vec2 sourcePos = sourceWarp->getPosition(); // Unused for now
        
        // Instead of keeping the same offset, teleport to a position slightly away from the destination warp
        // This prevents the ball from immediately triggering the destination warp
        float offsetDistance = sourceWarp->getRadius() * 1.5f;
        b2Vec2 destPos = destWarp->getPosition();
        
        // Calculate a random direction away from the destination warp center
        // Using a fixed angle for consistency
        float angle = 0.7f; // About 40 degrees
        b2Vec2 offsetDir = {cosf(angle), sinf(angle)};
        offsetDir = b2Normalize(offsetDir);
        
        // Calculate new position at the other warp with offset
        b2Vec2 scaledOffset = {offsetDir.x * offsetDistance, offsetDir.y * offsetDistance};
        b2Vec2 newPos = b2Add(destPos, scaledOffset);
        
        // Teleport the ball
        b2Rot rotation = {1.0f, 0.0f}; // Identity rotation (angle = 0)
        b2Body_SetTransform(ballBody, newPos, rotation);
        
        // Re-apply velocity
        b2Body_SetLinearVelocity(ballBody, velocity);
        
        // Set cooldown on both warps to prevent infinite loops
        sourceWarp->setCooldown(true);
        sourceWarp->cooldown_timer_ = WARP_COOLDOWN_TIME;
        destWarp->setCooldown(true);
        destWarp->cooldown_timer_ = WARP_COOLDOWN_TIME;
        
        std::cout << "Teleported ball from warp " << warpId << " to destination warp" << std::endl;
        std::cout << "Both warps are now on cooldown for " << WARP_COOLDOWN_TIME << " seconds" << std::endl;
    }
}

void Warp::render(SDL_Renderer* renderer, float camera_offset_x, float camera_offset_y, b2Vec2 rotated_position) const {
    if (!b2Body_IsValid(bodyId_)) {
        return; // Don't render if not valid
    }

    b2Vec2 current_pos;
    // Use rotated position if provided (non-zero), otherwise use body position
    // For warps, which are static, rotated_position will likely be {0,0} unless explicitly calculated and passed.
    if (rotated_position.x != 0.0f || rotated_position.y != 0.0f) {
        current_pos = rotated_position;
    } else {
        current_pos = b2Body_GetPosition(bodyId_);
    }
    
    // Convert physics position (meters) to screen position (pixels)
    int screen_x = static_cast<int>((current_pos.x * PPM) + camera_offset_x);
    int screen_y = static_cast<int>((current_pos.y * PPM) + camera_offset_y);
    int screen_radius = static_cast<int>(radius_ * PPM);
    
    SDL_Color color = getColor();
    if (isOnCooldown()) {
        color.a = 100; // Make semi-transparent if on cooldown
    }
    
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    DrawWarpCircle(renderer, screen_x, screen_y, screen_radius);
}
