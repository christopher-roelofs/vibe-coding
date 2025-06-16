#ifndef LEVEL_H
#define LEVEL_H

#include <vector>
#include <string>
#include <map>
#include <utility> // for std::pair
#include <box2d/box2d.h> // For b2Vec2

struct LevelData {
    std::vector<std::string> layout;
    int width;
    int height;
    b2Vec2 ball_start_position;
    b2Vec2 goal_position;
    std::vector<b2Vec2> hole_positions;
    std::vector<b2Vec2> reverse_item_positions;
    std::vector<std::pair<int, b2Vec2>> warp_positions; // <warp_id, position>
    std::string name;
    std::string description;
    std::string author;
    std::string difficulty;
};

class Level {
public:
    Level();
    bool loadFromFile(const std::string& filepath);
    bool loadNextLevel(); // Load the next level in the file
    bool loadLevelByIndex(int index); // Load a specific level by index
    
    // Getters for the current level
    const std::vector<std::string>& getLayout() const;
    int getWidth() const;
    int getHeight() const;
    b2Vec2 getBallStartPosition() const;
    b2Vec2 getGoalPosition() const;
    const std::vector<b2Vec2>& getHolePositions() const;
    const std::vector<b2Vec2>& getReverseItemPositions() const;
    const std::vector<std::pair<int, b2Vec2>>& getWarpPositions() const;
    std::string getName() const;
    std::string getDescription() const;
    std::string getAuthor() const;
    std::string getDifficulty() const;
    
    // Access the raw grid data
    const std::vector<std::string>& getGrid() const { return current_level_.layout; }
    
    // Level management
    int getCurrentLevelIndex() const { return current_level_index_; }
    const LevelData* getCurrentLevelData() const;
    int getTotalLevels() const { return levels_.size(); }
    bool hasMoreLevels() const { return current_level_index_ < levels_.size() - 1; }

private:
    // Parse a single level from the file
    bool parseLevelFromFile(std::ifstream& file, int startLineHint, LevelData& levelData);
    
    std::vector<LevelData> levels_; // All levels from the file
    LevelData current_level_; // Currently active level
    int current_level_index_; // Index of the current level
    std::string filepath_; // Store the filepath for reloading
};

#endif // LEVEL_H
