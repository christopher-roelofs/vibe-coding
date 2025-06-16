#include "Level.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cctype>

Level::Level() : current_level_index_(-1) {
    // Initialize with empty levels
}

bool Level::loadFromFile(const std::string& filepath) {
    std::cout << "Starting to load levels from file: " << filepath << std::endl;
    
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open level file: " << filepath << std::endl;
        return false;
    }
    
    // Store the filepath for potential reloading
    filepath_ = filepath;
    
    // Clear any existing levels
    levels_.clear();
    current_level_index_ = -1;
    
    // Read level pack metadata first (at the top of the file)
    std::string line;
    std::string level_pack_name;
    std::string level_pack_description;
    std::string level_pack_author;
    std::string level_pack_difficulty;
    std::string level_pack_date;
    
    // Process initial metadata until we find a section header or grid data
    bool found_section_header = false;
    std::streampos section_start_pos;
    
    // Remember the start position of the file
    std::streampos start_pos = file.tellg();
    
    while (std::getline(file, line) && !found_section_header) {
        // Remember position before reading this line
        std::streampos line_start_pos = file.tellg();
        line_start_pos -= line.length() + 1; // Adjust to the start of the line
        
        // Trim whitespace
        line.erase(0, line.find_first_not_of(" \t\n\r"));
        line.erase(line.find_last_not_of(" \t\n\r") + 1);
        
        if (line.empty()) {
            continue; // Skip empty lines
        }
        
        if (line[0] == ';') {
            std::string metadata = line.substr(1);
            metadata.erase(0, metadata.find_first_not_of(" \t"));
            
            if (metadata.find("Name: ") == 0) {
                // Check if this is the first Name: field we've encountered
                if (level_pack_name.empty()) {
                    level_pack_name = metadata.substr(6);
                    std::cout << "Level pack name: " << level_pack_name << std::endl;
                } else {
                    // If we already have a level pack name, this is the start of the first level
                    found_section_header = true;
                    section_start_pos = line_start_pos;
                    std::cout << "Found first level with name: '" << metadata << "'" << std::endl;
                }
            } else if (metadata.find("Description: ") == 0) {
                level_pack_description = metadata.substr(13);
                std::cout << "Level pack description: " << level_pack_description << std::endl;
            } else if (metadata.find("Author: ") == 0) {
                level_pack_author = metadata.substr(8);
                std::cout << "Level pack author: " << level_pack_author << std::endl;
            } else if (metadata.find("Date: ") == 0) {
                level_pack_date = metadata.substr(6);
                std::cout << "Level pack date: " << level_pack_date << std::endl;
            } else if (metadata.find("Difficulty: ") == 0) {
                level_pack_difficulty = metadata.substr(12);
                std::cout << "Level pack difficulty: " << level_pack_difficulty << std::endl;
            } else {
                // Any other comment line after the metadata is considered a level section header
                found_section_header = true;
                section_start_pos = line_start_pos;
                std::cout << "Found first section header: '" << metadata << "'" << std::endl;
            }
        } else {
            // Found grid data, rewind to start of this line
            found_section_header = true;
            section_start_pos = line_start_pos;
        }
    }
    
    // Rewind to the first section header or grid data
    file.clear();
    if (found_section_header) {
        file.seekg(section_start_pos);
    } else {
        // If no section header found, rewind to beginning
        file.seekg(start_pos);
    }
    
    // Parse all levels from the file
    int line_number = 1;
    LevelData level_data;
    int level_count = 0;
    
    while (file.good()) {
        std::cout << "Attempting to parse level at line " << line_number << std::endl;
        if (parseLevelFromFile(file, line_number, level_data)) {
            level_count++;
            // If level has no name, use level pack name + level number
            if (level_data.name.empty()) {
                level_data.name = level_pack_name + " - Level " + std::to_string(level_count);
            }
            std::cout << "Successfully parsed level " << level_count << ": " << level_data.name;
            if (!level_data.description.empty()) {
                std::cout << " - '" << level_data.description << "'";
            }
            if (!level_data.difficulty.empty()) {
                std::cout << " (Difficulty: " << level_data.difficulty << ")";
            }
            std::cout << " (" << level_data.width << "x" << level_data.height << ")" << std::endl;
            levels_.push_back(level_data);
            // Don't increment line_number anymore - parseLevelFromFile handles file position now
        } else {
            std::cout << "No more levels found or error occurred" << std::endl;
            break; // No more levels or error
        }
    }
    
    
    file.close();
    
    if (levels_.empty()) {
        std::cerr << "Error: No valid levels found in file: " << filepath << std::endl;
        return false;
    }
    
    // Load the first level
    return loadLevelByIndex(0);
}

bool Level::parseLevelFromFile(std::ifstream& file, int startLineHint, LevelData& levelData) {
    // Reset the level data
    levelData = LevelData();
    levelData.width = 0;
    levelData.height = 0;
    levelData.warp_positions.clear();
    
    std::cout << "parseLevelFromFile: Starting at line " << startLineHint << std::endl;
    
    std::string line;
    int file_line_counter = 0; // To keep track of actual lines read from file for accurate seeking

    // We're no longer using startLineHint to skip lines since we're tracking file position
    // directly with tellg/seekg. The startLineHint parameter is kept for backward compatibility
    // but we'll ignore it and just start parsing from the current file position.
    std::cout << "Starting to parse from current file position" << std::endl;

    // Process metadata and comments before the level grid
    bool found_level_data = false;
    bool in_grid_data = false;
    
    while (true) {
        // Remember position before reading the line
        std::streampos line_pos = file.tellg();
        
        if (!std::getline(file, line)) {
            break; // End of file
        }
        
        file_line_counter++;
        
        std::string original_line = line; // For seekg
        // Trim whitespace
        line.erase(0, line.find_first_not_of(" \t\n\r"));
        line.erase(line.find_last_not_of(" \t\n\r") + 1);

        // Skip empty lines if we haven't started grid data yet, or if it's between levels
        if (line.empty()) {
            if (in_grid_data) { // Empty line after grid data signifies end of current level
                break;
            }
            continue;
        }
        
        // Process comments and metadata
        if (line[0] == ';') {
            // If we were in grid data and now see a comment line, it's a new level section header
            if (in_grid_data) {
                 std::cout << "Found new section header while in grid data: '" << line << "'" << std::endl;
                 file.clear();
                 // Rewind to the beginning of this line
                 file.seekg(line_pos);
                 std::cout << "Rewinding file position to read this section in next call" << std::endl;
                 std::cout << "Completed parsing level with " << levelData.layout.size() << " grid lines" << std::endl;
                 return !levelData.layout.empty(); // Return true if we parsed something for the current level
            }

            std::string metadata = line.substr(1);
            metadata.erase(0, metadata.find_first_not_of(" \t"));
            
            if (metadata.find("Name: ") == 0) {
                levelData.name = metadata.substr(6);
                std::cout << "Found level name: '" << levelData.name << "'" << std::endl;
            } else if (metadata.find("Description: ") == 0) {
                levelData.description = metadata.substr(13);
                std::cout << "Found level description: '" << levelData.description << "'" << std::endl;
            } else if (metadata.find("Author: ") == 0) {
                levelData.author = metadata.substr(8);
                std::cout << "Found level author: '" << levelData.author << "'" << std::endl;
            } else if (metadata.find("Difficulty: ") == 0) {
                levelData.difficulty = metadata.substr(12);
                std::cout << "Found level difficulty: '" << levelData.difficulty << "'" << std::endl;
            } else {
                 std::cout << "Found section header: '" << metadata << "'" << std::endl;
                 // This is a section header (any comment line that isn't recognized metadata)
                 // If we already have grid data for the *current* level, it means this is the start of the *next* level.
                if (in_grid_data) {
                    std::cout << "Already in grid data, treating this as a new level section" << std::endl;
                    file.clear(); // Clear EOF flags if any
                    // We need to rewind to the beginning of THIS section header line so the next call to parseLevelFromFile can process it.
                    file.seekg(line_pos);
                    break; // End parsing for the current level
                } else {
                    std::cout << "Not in grid data yet, using as level name: '" << metadata << "'" << std::endl;
                    // If we haven't started grid data yet, use this as the level name if no explicit Name: tag is found
                    if (levelData.name.empty()) {
                        levelData.name = metadata;
                    }
                }
            }
            continue;
        }
        
        // If we reach here, we're processing the level grid data
        found_level_data = true;
        in_grid_data = true; // Mark that we are now in grid data mode
        
        // Add the line to the layout
        levelData.layout.push_back(line);
        
        // Update width if needed
        if (levelData.width == 0) {
            levelData.width = line.length();
        } else if (line.length() != levelData.width) {
            std::cerr << "Error: Level has inconsistent line widths. Expected " << levelData.width << ", got " << line.length() << std::endl;
            return false;
        }
        
        // Process the characters in the line
        for (int c = 0; c < line.length(); ++c) {
            switch (line[c]) {
                case 'O': // Ball start position
                    levelData.ball_start_position = {(float)c, (float)levelData.layout.size() - 1};
                    break;
                case 'G': // Goal
                    levelData.goal_position = {(float)c, (float)levelData.layout.size() - 1};
                    break;
                case 'H': // Hole
                    levelData.hole_positions.push_back({static_cast<float>(c), static_cast<float>(levelData.layout.size() - 1)});
                    break;
                case 'R': // Reverse item
                    levelData.reverse_item_positions.push_back({static_cast<float>(c), static_cast<float>(levelData.layout.size() - 1)});
                    break;
                case '1': // Warp (for now, only warp ID 1 is supported)
                    levelData.warp_positions.push_back({1, {static_cast<float>(c), static_cast<float>(levelData.layout.size() - 1)}});
                    break;
            }
        }
    }
    
    // Update height
    levelData.height = levelData.layout.size();

    // Check if we found any level data
    if (!found_level_data || levelData.width == 0 || levelData.height == 0) {
        return false;
    }
    
    static int level_counter = 0;
    std::cout << "Successfully parsed level " << ++level_counter << ": " << levelData.name;
    if (!levelData.description.empty()) {
        std::cout << " - '" << levelData.description << "'";
    }
    if (!levelData.difficulty.empty()) {
        std::cout << " (Difficulty: " << levelData.difficulty << ")";
    }
    std::cout << " (" << levelData.width << "x" << levelData.height << ")" << std::endl;
    return true;
}

bool Level::loadNextLevel() {
    if (hasMoreLevels()) {
        current_level_index_++;
        // This is the critical line that was missing. It copies the new level's data.
        current_level_ = levels_[current_level_index_]; 
        return true;
    }
    return false;
}

const LevelData* Level::getCurrentLevelData() const {
    if (current_level_index_ >= 0 && static_cast<size_t>(current_level_index_) < levels_.size()) {
        return &levels_[current_level_index_];
    }
    return nullptr;
}

bool Level::loadLevelByIndex(int index) {
    if (index < 0 || index >= levels_.size()) {
        return false;
    }
    
    current_level_ = levels_[index];
    current_level_index_ = index;
    
    std::cout << "Loaded level " << (index + 1) << "/" << levels_.size();
    if (!current_level_.name.empty()) {
        std::cout << ": " << current_level_.name;
    }
    std::cout << std::endl;
    
    return true;
}

const std::vector<std::string>& Level::getLayout() const {
    return current_level_.layout;
}

int Level::getWidth() const {
    return current_level_.width;
}

int Level::getHeight() const {
    return current_level_.height;
}

b2Vec2 Level::getBallStartPosition() const {
    return current_level_.ball_start_position;
}

b2Vec2 Level::getGoalPosition() const {
    return current_level_.goal_position;
}

const std::vector<b2Vec2>& Level::getHolePositions() const {
    return current_level_.hole_positions;
}

const std::vector<b2Vec2>& Level::getReverseItemPositions() const {
    return current_level_.reverse_item_positions;
}

const std::vector<std::pair<int, b2Vec2>>& Level::getWarpPositions() const {
    return current_level_.warp_positions;
}

std::string Level::getName() const {
    return current_level_.name;
}

std::string Level::getDescription() const {
    return current_level_.description;
}

std::string Level::getAuthor() const {
    return current_level_.author;
}

std::string Level::getDifficulty() const {
    return current_level_.difficulty;
}
