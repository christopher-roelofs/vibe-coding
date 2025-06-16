#include "Ball.h"
#include <iostream>
#include <cmath> // For M_PI, cos, sin

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Ball::Ball(b2WorldId worldId) : worldId_(worldId), bodyId_(b2_nullBodyId), radiusMeters_(0.0f) {
}

Ball::~Ball() {
    if (b2Body_IsValid(bodyId_)) {
        b2DestroyBody(bodyId_);
        bodyId_ = b2_nullBodyId;
    }
}

void Ball::create(b2Vec2 position_meters, float radius_meters) {
    if (b2Body_IsValid(bodyId_)) {
        b2DestroyBody(bodyId_); // Destroy old body if one exists
        bodyId_ = b2_nullBodyId;
    }
    radiusMeters_ = radius_meters;

    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = b2_dynamicBody;
    bodyDef.position = position_meters;
    bodyDef.isBullet = true; // Enable CCD to prevent clipping
    bodyDef.linearDamping = 0.1f; // Slight damping to prevent excessive sliding
    bodyDef.angularDamping = 0.2f; // Reduce spinning
    bodyDef.gravityScale = 1.5f; // Make ball more affected by gravity

    bodyId_ = b2CreateBody(worldId_, &bodyDef);
    if (!b2Body_IsValid(bodyId_)) {
        std::cerr << "Error: Failed to create ball body!" << std::endl;
        return;
    }
    b2Body_EnableSleep(bodyId_, false);

    b2Circle circleShape;
    circleShape.center = {0.0f, 0.0f}; // Relative to body's origin
    circleShape.radius = radiusMeters_;

    b2ShapeDef fixtureDef = b2DefaultShapeDef();
    fixtureDef.density = 2.0f; // Increased density for better collision
    fixtureDef.material.friction = 0.3f; // Increased friction to reduce sliding
    fixtureDef.material.restitution = 0.01f; // Almost no bounce (was 0.1f)

    b2CreateCircleShape(bodyId_, &fixtureDef, &circleShape);
}

void Ball::render(SDL_Renderer* renderer, float camera_offset_x, float camera_offset_y) const {
    if (!b2Body_IsValid(bodyId_)) return;

    b2Vec2 position_meters = b2Body_GetPosition(bodyId_);
    b2Rot body_rotation = b2Body_GetRotation(bodyId_);
    float angle_rad = b2Rot_GetAngle(body_rotation);

    float screen_x = position_meters.x * PPM + camera_offset_x;
    float screen_y = position_meters.y * PPM + camera_offset_y;
    float screen_radius = radiusMeters_ * PPM;

    // Draw a simple circle for the ball
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Red ball
    const int32_t segments = 20;
    const float_t increment = 2.0f * M_PI / segments;
    float_t theta = 0.0f;

    for (int i = 0; i < segments; ++i) {
        float x1 = screen_radius * cosf(theta) + screen_x;
        float y1 = screen_radius * sinf(theta) + screen_y;
        float x2 = screen_radius * cosf(theta + increment) + screen_x;
        float y2 = screen_radius * sinf(theta + increment) + screen_y;
        SDL_RenderDrawLine(renderer, static_cast<int>(x1), static_cast<int>(y1), static_cast<int>(x2), static_cast<int>(y2));
        theta += increment;
    }
    
    // Optional: Draw a line to indicate ball's rotation
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // White line
    float line_end_x = screen_x + screen_radius * cosf(angle_rad);
    float line_end_y = screen_y + screen_radius * sinf(angle_rad);
    SDL_RenderDrawLine(renderer, static_cast<int>(screen_x), static_cast<int>(screen_y), static_cast<int>(line_end_x), static_cast<int>(line_end_y));
}

void Ball::applyForceToCenter(const b2Vec2& force) {
    if (b2Body_IsValid(bodyId_)) {
        b2Body_ApplyForceToCenter(bodyId_, force, true);
    }
}

void Ball::reset(b2Vec2 position_meters) {
    if (b2Body_IsValid(bodyId_)) {
        b2Body_SetTransform(bodyId_, position_meters, b2MakeRot(0.0f));
        b2Body_SetLinearVelocity(bodyId_, {0.0f, 0.0f});
        b2Body_SetAngularVelocity(bodyId_, 0.0f);
    }
}

b2BodyId Ball::getBodyId() const {
    return bodyId_;
}

b2Vec2 Ball::getPosition() const {
    if (b2Body_IsValid(bodyId_)) {
        return b2Body_GetPosition(bodyId_);
    }
    return {0.0f, 0.0f}; // Should not happen if used correctly
}

b2Vec2 Ball::getVelocity() const {
    if (b2Body_IsValid(bodyId_)) {
        return b2Body_GetLinearVelocity(bodyId_);
    }
    return {0.0f, 0.0f}; // Should not happen if used correctly
}

float Ball::getAngle() const {
    if (b2Body_IsValid(bodyId_)) {
        b2Rot body_rotation = b2Body_GetRotation(bodyId_);
        return b2Rot_GetAngle(body_rotation);
    }
    return 0.0f;
}


