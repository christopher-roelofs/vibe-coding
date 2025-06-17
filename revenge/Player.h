#ifndef PLAYER_H
#define PLAYER_H

#include "GameObject.h"
#include <SDL2/SDL.h>

class Level; // Forward declaration

enum class MoveResult {
    SUCCESS,
    BLOCKED_WALL,
    BLOCKED_CAT,
    BLOCKED_TRAP,
    BLOCKED_CHAIN,
    SUCCESS_HOLE
};

class Player : public GameObject {
public:
    Player(int x, int y, int width, int height);

    void render(SDL_Renderer* renderer, int offsetX = 0, int offsetY = 0) override;
    MoveResult move(int dx, int dy, Level& level);
    void update(Level& level) override; // Player-specific update logic if any

    void setLives(int lives) { m_lives = lives; }
    void decrementLife() { m_lives--; } // Use this instead of loseLife
    int getScore() const;
    void setScore(int score);
    void addScore(int points);
    int getLives() const;
    bool isStuck() const;

private:
    int m_lives;
    Uint32 m_stuckUntil = 0;
    int m_score; // Player's score
};

#endif // PLAYER_H
