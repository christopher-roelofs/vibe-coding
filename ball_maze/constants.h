#ifndef CONSTANTS_H
#define CONSTANTS_H

// Screen dimensions
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

// Tile size (pixels)
const int TILE_SIZE = 40;

// Box2D scaling
const float PPM = 32.0f; // Pixels Per Meter

// Physics simulation parameters
const float MAZE_TARGET_ROTATION_SPEED_DPS = 45.0f; // Target rotation speed in degrees per second (reduced from 90.0f)
const float TIME_STEP = 1.0f / 120.0f; // 120 FPS physics for stable simulation
const int VELOCITY_ITERATIONS = 20; // Increased to match POSITION_ITERATIONS (was 16)
const int POSITION_ITERATIONS = 20; // Further increased for collision accuracy (was 12)
const float MAZE_ANIMATION_ROTATION_SPEED_RAD_PER_SEC = M_PI / 2.0f; // Was M_PI (180dps), now 90dps. Aligns better with max physical rotation.
const float PHYSICS_SHAPE_OVERLAP_METERS = 0.0f;  // Physics shapes match tile size (was 0.05f, which made them smaller)
const float DISCRETE_ROTATION_STEP_RAD = M_PI / 180.0f * 0.1f; // Drastically reduced step for collision stability (0.1 degrees, was 0.25)

#endif // CONSTANTS_H
