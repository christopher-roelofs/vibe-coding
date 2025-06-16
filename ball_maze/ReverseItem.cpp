#include "ReverseItem.h"
#include <iostream>
#include <cmath> // For M_PI, sin, cos
#include <algorithm> // For std::max

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

const float ReverseItem::MAX_COOLDOWN_SECONDS = 5.0f;

// Helper function to draw a circle (from Game.cpp, potentially move to a graphics utility)
// For now, we'll define a simple one here if not available globally.
// If you have SDL_gfx, using its functions would be better for anti-aliasing.
void DrawCircle(SDL_Renderer* renderer, int center_x, int center_y, int radius) {
    for (int w = 0; w < radius * 2; w++) {
        for (int h = 0; h < radius * 2; h++) {
            int dx = radius - w;
            int dy = radius - h;
            if ((dx * dx + dy * dy) <= (radius * radius)) {
                // For a hollow circle, check if it's near the edge
                // This is a simple filled circle. For hollow, we need a thickness check.
                // SDL_RenderDrawPoint(renderer, center_x + dx, center_y + dy);
            }
        }
    }
    // More precise circle drawing using lines (SDL_RenderDrawCircle equivalent if not available)
    const int32_t segments = 24;
    const float increment = (float)(2.0 * M_PI / segments);
    float theta = 0.0f;
    for (int i = 0; i < segments; ++i) {
        float x1 = radius * cosf(theta) + center_x;
        float y1 = radius * sinf(theta) + center_y;
        theta += increment;
        float x2 = radius * cosf(theta) + center_x;
        float y2 = radius * sinf(theta) + center_y;
        SDL_RenderDrawLine(renderer, (int)x1, (int)y1, (int)x2, (int)y2);
    }
}

void DrawFilledPie(SDL_Renderer* renderer, int center_x, int center_y, int radius, float start_angle_rad, float end_angle_rad, SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    const int segments = std::max(1, static_cast<int>(radius * std::abs(end_angle_rad - start_angle_rad) / (M_PI / 12))); // Dynamic segments
    float angle_step = (end_angle_rad - start_angle_rad) / segments;

    for (int i = 0; i <= segments; ++i) {
        float angle1 = start_angle_rad + i * angle_step;
        float angle2 = start_angle_rad + (i + 1) * angle_step;
        if (i == segments) angle2 = end_angle_rad; // Ensure exact end angle

        // Triangle: center, point on arc 1, point on arc 2
        SDL_Vertex vertices[3] = {
            { { (float)center_x, (float)center_y }, color, { 0, 0 } },
            { { center_x + radius * cosf(angle1), center_y + radius * sinf(angle1) }, color, { 0, 0 } },
            { { center_x + radius * cosf(angle2), center_y + radius * sinf(angle2) }, color, { 0, 0 } }
        };
        SDL_RenderGeometry(renderer, nullptr, vertices, 3, nullptr, 0);
    }
}

ReverseItem::ReverseItem(b2WorldId worldId) : 
    worldId_(worldId), 
    bodyId_(b2_nullBodyId), 
    sizeMeters_(0.0f),
    active_(true),
    current_cooldown_seconds_(0.0f) {
    // Set a distinct color for the reverse item (purple)
    color_ = {255, 0, 255, 255}; // RGBA
}

ReverseItem::~ReverseItem() {
    if (b2Body_IsValid(bodyId_)) {
        b2DestroyBody(bodyId_);
        bodyId_ = b2_nullBodyId;
    }
}

void ReverseItem::create(b2Vec2 position_meters, float size_meters) {
    if (b2Body_IsValid(bodyId_)) {
        b2DestroyBody(bodyId_);
    }

    sizeMeters_ = size_meters;

    // Create a static body for the reverse item
    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = b2_staticBody;
    bodyDef.position = position_meters;
    bodyId_ = b2CreateBody(worldId_, &bodyDef);

    if (!b2Body_IsValid(bodyId_)) {
        std::cerr << "Error: Failed to create reverse item body!" << std::endl;
        return;
    }

    // Create a circular shape for the reverse item
    b2Circle circleShape;
    circleShape.center = {0.0f, 0.0f}; // Local coordinates
    circleShape.radius = sizeMeters_ / 2.0f;

    b2ShapeDef fixtureDef = b2DefaultShapeDef();
    fixtureDef.density = 0.0f; // Static body
    fixtureDef.material.friction = 0.0f; // No friction
    fixtureDef.material.restitution = 0.0f; // No bounce
    fixtureDef.isSensor = true; // Make it a sensor so it doesn't affect physics

    b2CreateCircleShape(bodyId_, &fixtureDef, &circleShape);
}

void ReverseItem::startCooldown() {
    current_cooldown_seconds_ = MAX_COOLDOWN_SECONDS;
}

bool ReverseItem::isCoolingDown() const {
    return current_cooldown_seconds_ > 0.0f;
}

void ReverseItem::updateCooldown(float delta_time) {
    if (current_cooldown_seconds_ > 0.0f) {
        current_cooldown_seconds_ -= delta_time;
        if (current_cooldown_seconds_ < 0.0f) {
            current_cooldown_seconds_ = 0.0f;
        }
    }
}

float ReverseItem::getCooldownPercentage() const {
    if (MAX_COOLDOWN_SECONDS <= 0.0f) return 1.0f; // Avoid division by zero, fully cooled down
    return std::max(0.0f, 1.0f - (current_cooldown_seconds_ / MAX_COOLDOWN_SECONDS));
}

float ReverseItem::getRadius() const {
    return sizeMeters_ / 2.0f;
}

void ReverseItem::render(SDL_Renderer* renderer, float camera_offset_x, float camera_offset_y, b2Vec2 rotated_position) const {
    if (!b2Body_IsValid(bodyId_) || !active_) {
        return; // Don't render if not valid or not active
    }

    b2Vec2 position;
    // Use rotated position if provided (non-zero), otherwise use body position
    if (rotated_position.x != 0.0f || rotated_position.y != 0.0f) {
        position = rotated_position;
    } else {
        position = b2Body_GetPosition(bodyId_);
    }
    
    // Convert physics position (meters) to screen position (pixels)
    int screen_x = static_cast<int>((position.x * PPM) + camera_offset_x);
    int screen_y = static_cast<int>((position.y * PPM) + camera_offset_y);
    int screen_radius = static_cast<int>(sizeMeters_ * PPM / 2.0f);
    
    // Set the draw color to the reverse item color
    // Draw the outer empty circle (outline)
    SDL_SetRenderDrawColor(renderer, color_.r, color_.g, color_.b, color_.a); // Use item's main color for outline
    DrawCircle(renderer, screen_x, screen_y, screen_radius);

    if (isCoolingDown()) {
        float percentage = getCooldownPercentage();
        float fill_angle_rad = percentage * 2.0f * M_PI; // Fill from 0 to percentage * 360 degrees

        // Cooldown fill color (e.g., a lighter or semi-transparent version of item color, or a distinct color)
        SDL_Color cooldown_fill_color = {128, 0, 128, 150}; // Semi-transparent purple for cooldown
        if (color_.r == 128 && color_.g == 0 && color_.b == 128) { // If main color is already purple, pick another
            cooldown_fill_color = {0, 128, 128, 150}; // Semi-transparent teal
        }
        
        // Draw filled pie segment. Start angle is typically 0 (or -M_PI/2 for top-start)
        // Let's make it start from the top (-M_PI/2) and go clockwise.
        DrawFilledPie(renderer, screen_x, screen_y, screen_radius -1, -M_PI / 2.0f, (-M_PI / 2.0f) + fill_angle_rad, cooldown_fill_color);
    }
}

b2BodyId ReverseItem::getBodyId() const {
    return bodyId_;
}

b2Vec2 ReverseItem::getPosition() const {
    if (b2Body_IsValid(bodyId_)) {
        return b2Body_GetPosition(bodyId_);
    }
    return {0.0f, 0.0f};
}

bool ReverseItem::isActive() const {
    return active_;
}

void ReverseItem::reset() {
    active_ = true;
    current_cooldown_seconds_ = 0.0f;
    // Reset color to original purple
    color_ = {128, 0, 128, 255};
}

void ReverseItem::deactivate() {
    active_ = false;
}
