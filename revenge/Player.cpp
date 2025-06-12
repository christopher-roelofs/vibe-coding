#include "Player.h"
#include "Debug.h"
#include "TextureManager.h"
#include "Level.h"
#include "Block.h"
#include "Cat.h"
#include "Cheese.h" // Added for destroying cheese with blocks
#include "Trap.h"
#include "Hole.h"
#include <vector>          // For collision detection


Player::Player(int x, int y, int width, int height)
    : GameObject(x, y, width, height, "player", "mouse"), m_lives(3), m_score(0) { // "player" is tag, "mouse" is textureID
}

void Player::render(SDL_Renderer* renderer, int offsetX, int offsetY) {
    if (isStuck()) {
        return; // Player is in a hole, don't render
    }
    // Convert grid coordinates to pixel coordinates for rendering
    TextureManager::draw("mouse", m_x * m_width, m_y * m_height, m_width, m_height, renderer, offsetX, offsetY);
}

MoveResult Player::move(int dx, int dy, Level& level) {
    if (g_debugMode) {
        std::cout << "Player attempting to move by (" << dx << ", " << dy << ") from (" << m_x << ", " << m_y << ")" << std::endl;
    }

    if (isStuck()) {
        return MoveResult::BLOCKED_WALL; // Player is stuck, can't move
    }

    int newX = m_x + dx;
    int newY = m_y + dy;

    const int POINTS_PER_CHEESE = 50;

    // 1. Check for walls
    if (level.isTileSolid(newX, newY)) {
        return MoveResult::BLOCKED_WALL;
    }

    GameObject* targetObject = level.getGameObjectAt(newX, newY);

    // 2. Check for Cat or Trap collision
    if (dynamic_cast<Cat*>(targetObject)) {
        decrementLife();
        return MoveResult::BLOCKED_CAT;
    }
    if (dynamic_cast<Trap*>(targetObject)) {
        m_lives--;
        return MoveResult::BLOCKED_TRAP;
    }
    if (dynamic_cast<Hole*>(targetObject)) {
        m_stuckUntil = SDL_GetTicks() + 1000;
        m_x = newX;
        m_y = newY;
        return MoveResult::SUCCESS_HOLE;
    }

    // 3. Handle empty space
    if (targetObject == nullptr) {
        m_x = newX;
        m_y = newY;
        return MoveResult::SUCCESS;
    }

    // 4. Handle collecting cheese
    if (targetObject->getTag() == "cheese") {
        level.removeGameObjectAt(newX, newY);
        level.decrementCheeseCount();
        m_score += POINTS_PER_CHEESE;
        m_x = newX;
        m_y = newY;
        return MoveResult::SUCCESS;
    }

    // 5. Handle pushing blocks
    if (auto* firstBlock = dynamic_cast<Block*>(targetObject)) {
        std::vector<GameObject*> push_chain;
        push_chain.push_back(firstBlock);

        int current_check_x = firstBlock->getX() + dx;
        int current_check_y = firstBlock->getY() + dy;

        while (true) {
            if (level.isTileSolid(current_check_x, current_check_y)) {
                return MoveResult::BLOCKED_CHAIN;
            }

            GameObject* nextObject = level.getGameObjectAt(current_check_x, current_check_y);

            if (nextObject == nullptr) {
                break; // End of chain, push is valid
            }

            if (auto* nextBlock = dynamic_cast<Block*>(nextObject)) {
                push_chain.push_back(nextBlock);
                current_check_x += dx;
                current_check_y += dy;
                continue;
            }

            if (auto* cat = dynamic_cast<Cat*>(nextObject)) {
                int space_behind_cat_x = current_check_x + dx;
                int space_behind_cat_y = current_check_y + dy;
                if (level.isTileSolid(space_behind_cat_x, space_behind_cat_y) || level.getGameObjectAt(space_behind_cat_x, space_behind_cat_y) != nullptr) {
                    return MoveResult::BLOCKED_CHAIN;
                }
                push_chain.push_back(cat);
                break;
            }
            
            if (auto* cheese = dynamic_cast<Cheese*>(nextObject)) {
                level.removeGameObjectAt(cheese->getX(), cheese->getY());
                level.decrementCheeseCount();
                break; // Space is now clear
            }

            return MoveResult::BLOCKED_CHAIN; // Any other object blocks the push
        }

        // If the loop completes, the push is valid. Move the chain.
        for (auto it = push_chain.rbegin(); it != push_chain.rend(); ++it) {
            GameObject* obj_to_move = *it;
            obj_to_move->setPosition(obj_to_move->getX() + dx, obj_to_move->getY() + dy);
        }

        m_x = newX;
        m_y = newY;
        return MoveResult::SUCCESS;
    }

    return MoveResult::BLOCKED_CHAIN; // Default case: something unhandled is blocking
}

void Player::addScore(int points) { m_score += points; }

void Player::update(Level& level) {
    // Player-specific update logic can go here
}

int Player::getScore() const {
    return m_score;
}

int Player::getLives() const {
    return m_lives;
}

bool Player::isStuck() const {
    return SDL_GetTicks() < m_stuckUntil;
}

void Player::setScore(int score) {
    m_score = score;
}
