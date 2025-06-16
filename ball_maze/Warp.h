#ifndef WARP_H
#define WARP_H

#include <box2d/box2d.h>
#include <SDL2/SDL.h>
#include <vector>
#include <memory>



class Warp {
public:
    Warp(b2WorldId worldId, int id, b2Vec2 position, float radius = 0.2f);
    ~Warp();
    
    b2BodyId getBodyId() const { return bodyId_; }
    int getId() const { return id_; }
    b2Vec2 getPosition() const;
    b2Vec2 getOriginalPosition() const { return position_; } // Return the original unrotated position
    float getRadius() const { return radius_; }
    SDL_Color getColor() const;
    
    // Cooldown management
    bool isOnCooldown() const { return is_on_cooldown_; }
    void setCooldown(bool cooldown) { is_on_cooldown_ = cooldown; }
    void updateCooldown(float delta_time);
    void resetCooldown() { is_on_cooldown_ = false; cooldown_timer_ = 0.0f; }
    
    // Static method to handle warping between pairs
    static void handleWarpCollision(int warpId, b2BodyId ballBody, const std::vector<std::unique_ptr<Warp>>& warps);
    
    void render(SDL_Renderer* renderer, float camera_offset_x, float camera_offset_y, b2Vec2 rotated_position = {0.0f, 0.0f}) const;

    // Cooldown constants
    static constexpr float WARP_COOLDOWN_TIME = 2.0f; // seconds
    
private:
    b2WorldId worldId_;
    b2BodyId bodyId_;
    int id_;
    b2Vec2 position_;
    float radius_;
    
    // Cooldown state
    bool is_on_cooldown_ = false;
    float cooldown_timer_ = 0.0f;
    
    // Disable copying
    Warp(const Warp&) = delete;
    Warp& operator=(const Warp&) = delete;
};

#endif // WARP_H
