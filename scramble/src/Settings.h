#ifndef SETTINGS_H
#define SETTINGS_H

#include <string>
#include <SDL.h>
#include <map>

class Settings {
public:
    Settings();
    bool load(const std::string& filename);

    SDL_Color getColor(const std::string& key, SDL_Color default_color = {255, 255, 255, 255}) const;
    std::string getFontPath() const; // Returns filename, e.g., VCR_OSD_MONO.ttf

private:
    std::map<std::string, std::string> color_settings_;
    std::string font_path_ = "VCR_OSD_MONO.ttf"; // Default font filename

    SDL_Color hexToSdlColor(const std::string& hex) const;
    void trim(std::string& s) const;
};

#endif // SETTINGS_H
