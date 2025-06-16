#ifndef BALL_H
#define BALL_H

#include <SDL2/SDL.h>
#include <box2d/box2d.h>
#include "constants.h"

class Ball {
public:
    Ball(b2WorldId worldId);
    ~Ball();

    void create(b2Vec2 position_meters, float radius_meters);
    void render(SDL_Renderer* renderer, float camera_offset_x, float camera_offset_y) const;
    void applyForceToCenter(const b2Vec2& force);
    void reset(b2Vec2 position_meters);

    b2BodyId getBodyId() const;
    b2Vec2 getPosition() const; // In meters
    b2Vec2 getVelocity() const; // In meters/second
    float getAngle() const;    // In radians
    float getRadius() const { return radiusMeters_; } // Return the ball radius

private:
    b2WorldId worldId_;
    b2BodyId bodyId_;
    float radiusMeters_;
    // SDL_Texture* texture_; // Optional: for sprite-based rendering
};

#endif // BALL_H
