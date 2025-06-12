#ifndef LEVEL_H
#define LEVEL_H

#include <string>
#include <vector>
#include <SDL2/SDL.h>
#include <memory> // For std::unique_ptr
#include <vector>
#include "GameObject.h"
class Player; // Forward-declare Player to break circular dependency

class Level {
public:
    Level();
    ~Level();

    bool load(const std::string& filename, int startLineHint = 1); // Default to 1 for old single-level files
    void render(SDL_Renderer* renderer, int offsetX = 0, int offsetY = 0);
    void resetAllPositions();

    int getWidth() const;
    int getHeight() const;
    int getTileSize() const;

    bool isTileSolid(int x, int y) const;
    GameObject* getGameObjectAt(int x, int y) const;
    Player* getPlayer() const;
    const std::vector<std::unique_ptr<GameObject>>& getGameObjects() const;

    void removeGameObjectAt(int x, int y);
    void updateTrappedCats();
    int getCatCount() const;
    int getCheeseCount() const;
    void decrementCatCount();
    void decrementCheeseCount();

private:
    int m_width, m_height, m_tileSize;
    std::vector<std::vector<char>> m_levelData;
    std::vector<std::unique_ptr<GameObject>> m_gameObjects;
    Player* m_player;
    int m_catCount;
    int m_cheeseCount;
};

#endif // LEVEL_H
