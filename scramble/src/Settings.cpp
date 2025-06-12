#include "Settings.h"
#include <fstream>
#include <sstream>
#include <algorithm> // For std::remove, std::transform
#include <iostream> // For error reporting

Settings::Settings() {}

void Settings::trim(std::string& s) const {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

SDL_Color Settings::hexToSdlColor(const std::string& hex) const {
    std::string c = hex;
    if (c.length() > 0 && c[0] == '#') {
        c = c.substr(1);
    }
    if (c.length() != 6) {
        std::cerr << "Warning: Invalid hex color format: " << hex << ". Using white." << std::endl;
        return {255, 255, 255, 255}; // Default to white
    }
    try {
        Uint8 r = static_cast<Uint8>(std::stoul(c.substr(0, 2), nullptr, 16));
        Uint8 g = static_cast<Uint8>(std::stoul(c.substr(2, 2), nullptr, 16));
        Uint8 b = static_cast<Uint8>(std::stoul(c.substr(4, 2), nullptr, 16));
        return {r, g, b, 255};
    } catch (const std::invalid_argument& ia) {
        std::cerr << "Warning: Invalid hex color string (std::invalid_argument): " << hex << ". Using white. Error: " << ia.what() << std::endl;
        return {255, 255, 255, 255};
    } catch (const std::out_of_range& oor) {
        std::cerr << "Warning: Hex color string out of range (std::out_of_range): " << hex << ". Using white. Error: " << oor.what() << std::endl;
        return {255, 255, 255, 255};
    }
}

bool Settings::load(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open settings file: " << filename << std::endl;
        return false;
    }

    std::string line, current_section;
    while (std::getline(file, line)) {
        trim(line);
        if (line.empty() || line[0] == ';') { // Skip empty lines and comments
            continue;
        }

        if (line[0] == '[' && line.back() == ']') {
            current_section = line.substr(1, line.length() - 2);
            std::transform(current_section.begin(), current_section.end(), current_section.begin(), ::tolower);
        } else {
            size_t delimiter_pos = line.find('=');
            if (delimiter_pos != std::string::npos) {
                std::string key = line.substr(0, delimiter_pos);
                std::string value = line.substr(delimiter_pos + 1);
                trim(key);
                trim(value);
                std::transform(key.begin(), key.end(), key.begin(), ::tolower);

                if (current_section == "colors") {
                    color_settings_[key] = value;
                } else if (current_section == "font") {
                    if (key == "font") {
                        font_path_ = value;
                    }
                }
            }
        }
    }
    file.close();
    return true;
}

SDL_Color Settings::getColor(const std::string& key, SDL_Color default_color) const {
    std::string lower_key = key;
    std::transform(lower_key.begin(), lower_key.end(), lower_key.begin(), ::tolower);
    auto it = color_settings_.find(lower_key);
    if (it != color_settings_.end()) {
        return hexToSdlColor(it->second);
    }
    std::cerr << "Warning: Color key '" << key << "' not found in settings. Using default." << std::endl;
    return default_color;
}

std::string Settings::getFontPath() const {
    return font_path_; // This now returns just the filename, e.g., "VCR_OSD_MONO.ttf"
}
