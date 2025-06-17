#include "Level.h"
#include "Player.h" // Include Player definition for implementation
#include "TextureManager.h"
#include "Block.h"
#include "Cat.h"
#include "Cheese.h"
#include "Trap.h"
#include "Hole.h"
#include <fstream>
#include <iostream>
#include <algorithm>

Level::Level() : m_width(0), m_height(0), m_tileSize(32), m_player(nullptr), m_catCount(0), m_cheeseCount(0) {}

Level::~Level() {} // GameObjects are now managed by unique_ptrs in a vector, so cleanup is automatic.

bool Level::load(const std::string& filename, int startLineHint) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open level file " << filename << std::endl;
        return false;
    }

    m_levelData.clear();
    m_gameObjects.clear();
    m_player = nullptr;
    m_catCount = 0;
    m_cheeseCount = 0;

    std::string line;
    int y_coord = 0;
    int current_line_number = 0;

    // Skip to the startLineHint
    while (current_line_number < startLineHint -1 && std::getline(file, line)) {
        current_line_number++;
    }

    // Now read the actual level data
    while (std::getline(file, line)) {
        current_line_number++;

        // Trim whitespace for checking if line is empty or a comment
        std::string trimmed_line = line;
        trimmed_line.erase(0, trimmed_line.find_first_not_of(" \t\n\r"));
        trimmed_line.erase(trimmed_line.find_last_not_of(" \t\n\r") + 1);

        // Stop conditions for the current level's grid data:
        // 1. Empty line after at least one row has been read.
        // 2. A new level definition comment.
        if (trimmed_line.empty() && !m_levelData.empty()) {
            break; // End of current level data (blank line separator)
        }
        if (trimmed_line.rfind("; Level ", 0) == 0 && !m_levelData.empty()) {
            break; // Start of a new level definition
        }
        if (trimmed_line.rfind(";", 0) == 0) { // Skip any other comment lines
            continue;
        }
        if (trimmed_line.empty()) { // Skip blank lines before actual level data starts
            continue;
        }

        std::vector<char> row;
        for (int x_coord = 0; x_coord < line.length(); ++x_coord) {
            char tile = line[x_coord];

            if (tile == 'M') {
                auto player = std::make_unique<Player>(x_coord, y_coord, m_tileSize, m_tileSize);
                m_player = player.get(); // Get raw pointer
                m_gameObjects.push_back(std::move(player));
                row.push_back('.'); // The player object now represents the mouse
            } else if (tile == 'B') {
                m_gameObjects.push_back(std::make_unique<Block>(x_coord, y_coord, m_tileSize, m_tileSize));
                row.push_back('.'); // The block object now represents the block
            } else if (tile == 'C') { // 'C' is Cheese
                m_gameObjects.push_back(std::make_unique<Cheese>(x_coord, y_coord, m_tileSize, m_tileSize));
                m_cheeseCount++;
                row.push_back('.'); // The cheese object now represents the cheese
            } else if (tile == 'K') { // 'K' is now for Cat
                m_gameObjects.push_back(std::make_unique<Cat>(x_coord, y_coord, m_tileSize, m_tileSize));
                m_catCount++;
                row.push_back('.'); // The cat object now represents the cat
            } else if (tile == 'T') { // 'T' is for Trap
                m_gameObjects.push_back(std::make_unique<Trap>(x_coord, y_coord, m_tileSize));
                row.push_back('.'); // The trap object now represents the trap
            } else if (tile == 'H') { // 'H' is for Hole
                m_gameObjects.push_back(std::make_unique<Hole>(x_coord, y_coord, m_tileSize));
                row.push_back('.'); // The hole object now represents the hole
            } else {
                row.push_back(tile);
            }
        }
        m_levelData.push_back(row);
        y_coord++;
    }

    if (!m_levelData.empty()) {
        m_height = m_levelData.size();
        m_width = m_levelData[0].size();
    } else {
        m_height = 0;
        m_width = 0;
    }
    
    file.close();
    return true;
}

void Level::render(SDL_Renderer* renderer, int offsetX, int offsetY) {
    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            // Always draw the background tile first
            TextureManager::draw("empty", x * m_tileSize, y * m_tileSize, m_tileSize, m_tileSize, renderer, offsetX, offsetY);

            char tileType = m_levelData[y][x];
            std::string textureId;

            switch (tileType) {
                case 'W':
                    textureId = "wall";
                    break;
                // No default case needed, as we've already drawn the background
            }
            
            if (!textureId.empty()) {
                 TextureManager::draw(textureId, x * m_tileSize, y * m_tileSize, m_tileSize, m_tileSize, renderer, offsetX, offsetY);
            }
        }
    }
    // Render all game objects
    for (const auto& obj : m_gameObjects) {
        obj->render(renderer, offsetX, offsetY);
    }
}

void Level::resetAllPositions() {
    for (auto& obj : m_gameObjects) {
        obj->resetPosition();
    }
}

bool Level::isTileSolid(int gridX, int gridY) const {
    if (gridY < 0 || gridX < 0 || gridY >= m_levelData.size() || gridX >= m_levelData[gridY].size()) {
        return true; // Out of bounds is solid
    }

    // Check for wall tiles
    if (m_levelData[gridY][gridX] == 'W') return true;

    return false;
}

Player* Level::getPlayer() const {
    return m_player;
}

GameObject* Level::getGameObjectAt(int x, int y) const {
    for (const auto& obj : m_gameObjects) {
        if (obj->getX() == x && obj->getY() == y) {
            return obj.get();
        }
    }
    return nullptr;
}

int Level::getWidth() const {
    return m_width;
}

int Level::getHeight() const {
    return m_height;
}

const std::vector<std::unique_ptr<GameObject>>& Level::getGameObjects() const {
    return m_gameObjects;
}

void Level::removeGameObjectAt(int x, int y) {
    m_gameObjects.erase(
        std::remove_if(m_gameObjects.begin(), m_gameObjects.end(),
                       [&](const std::unique_ptr<GameObject>& obj) {
                           return obj->getX() == x && obj->getY() == y;
                       }),
        m_gameObjects.end());
}

int Level::getCatCount() const {
    return m_catCount;
}

int Level::getCheeseCount() const {
    return m_cheeseCount;
}

void Level::decrementCatCount() {
    if (m_catCount > 0) {
        m_catCount--;
    }
}

void Level::decrementCheeseCount() {
    m_cheeseCount--;
}

int Level::getTileSize() const {
    return m_tileSize;
}

void Level::updateTrappedCats() {
    const int POINTS_PER_CAT_TRAP = 100;
    std::vector<GameObject*> catsToReplace;

    // Find all cats that are trapped
    for (auto& obj : m_gameObjects) {
        Cat* cat = dynamic_cast<Cat*>(obj.get());
        if (!cat) continue;

        int x = cat->getX();
        int y = cat->getY();

        // Check all 8 surrounding tiles (including diagonals)
        bool isTrapped = true;
        int directions[8][2] = {
            {0, -1}, {0, 1}, {-1, 0}, {1, 0}, // Cardinal
            {-1, -1}, {-1, 1}, {1, -1}, {1, 1}  // Diagonal
        };

        for (auto& dir : directions) {
            int checkX = x + dir[0];
            int checkY = y + dir[1];

            GameObject* blockingObject = getGameObjectAt(checkX, checkY);
            // A space is considered blocked if it's a solid wall tile OR if it's occupied by a block.
            bool isSpaceBlocked = isTileSolid(checkX, checkY) || (blockingObject && dynamic_cast<Block*>(blockingObject));

            if (!isSpaceBlocked) {
                isTrapped = false;
                break;
            }
        }

        if (isTrapped) {
            catsToReplace.push_back(cat);
        }
    }

    // Replace trapped cats with cheese
    for (auto* cat : catsToReplace) {
        int x = cat->getX();
        int y = cat->getY();

        // Remove the cat
        removeGameObjectAt(x, y);
        decrementCatCount();

        // Add cheese in its place
        m_gameObjects.push_back(std::make_unique<Cheese>(x, y, m_tileSize, m_tileSize));
        m_cheeseCount++;

        // Award points to the player
        if (m_player) {
            m_player->addScore(POINTS_PER_CAT_TRAP);
        }
    }
}
