#ifndef REVERSE_ITEM_H
#define REVERSE_ITEM_H

#include <SDL2/SDL.h>
#include <box2d/box2d.h>
#include "constants.h"

class ReverseItem {
public:
    static const float MAX_COOLDOWN_SECONDS;

    ReverseItem(b2WorldId worldId);
    ~ReverseItem();

    void create(b2Vec2 position_meters, float size_meters);
    void render(SDL_Renderer* renderer, float camera_offset_x, float camera_offset_y, b2Vec2 rotated_position = {0, 0}) const;
    
    b2BodyId getBodyId() const;
    b2Vec2 getPosition() const; // In meters
    
    // Check if the item is active (not yet collected/triggered)
    bool isActive() const;
    
    // Check if the item is currently cooling down
    bool isCoolingDown() const;
    
    // Start the cooldown for this item
    void startCooldown();
    
    // Update the cooldown timer
    void updateCooldown(float delta_time);

    // Get cooldown progress as a percentage (0.0 to 1.0)
    float getCooldownPercentage() const;

    // Get the radius of the item in meters
    float getRadius() const;

    // Reset the item so it can be triggered again (resets cooldown)
    void reset();
    
    // Deactivate the item (no longer has effect)
    void deactivate();

private:
    b2WorldId worldId_;
    b2BodyId bodyId_;
    float sizeMeters_;
    bool active_;    // Whether the item is active and can be collected
    float current_cooldown_seconds_; // Time remaining for cooldown
    
    // Color for rendering
    SDL_Color color_;
};

#endif // REVERSE_ITEM_H
