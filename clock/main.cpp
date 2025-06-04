#include <SDL2/SDL_ttf.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <algorithm>
#include <dirent.h>

struct SDLColorEqual {
    bool operator()(const SDL_Color &a, const SDL_Color &b) const {
        return a.r == b.r && a.g == b.g && a.b == b.b && a.a == b.a;
    }
};

std::string to_hex(SDL_Color color) {
    std::ostringstream oss;
    oss << std::hex << std::setw(2) << std::setfill('0') << (int)color.r
        << std::setw(2) << (int)color.g
        << std::setw(2) << (int)color.b;
    return oss.str();
}

SDL_Color from_hex(const std::string &hex) {
    SDL_Color color = {0, 0, 0, 255};
    if (hex.size() == 6 || (hex.size() == 7 && hex[0] == '#')) {
        std::string h = (hex[0] == '#') ? hex.substr(1) : hex;
        color.r = std::stoi(h.substr(0, 2), nullptr, 16);
        color.g = std::stoi(h.substr(2, 2), nullptr, 16);
        color.b = std::stoi(h.substr(4, 2), nullptr, 16);
    }
    return color;
}

std::string get_time_in_zone(const std::string &timezone, int format, bool showSeconds) {
    std::string oldTZ = getenv("TZ") ? getenv("TZ") : "";
    setenv("TZ", timezone.c_str(), 1);
    tzset();

    std::time_t t = std::time(nullptr);
    std::tm *now = std::localtime(&t);

    std::ostringstream oss;
    if (format == 12)
        oss << std::put_time(now, showSeconds ? "%I:%M:%S %p" : "%I:%M %p");
    else
        oss << std::put_time(now, showSeconds ? "%H:%M:%S" : "%H:%M");

    setenv("TZ", oldTZ.c_str(), 1);
    tzset();
    return oss.str();
}

// Function to get list of available fonts
std::vector<std::string> get_available_fonts() {
    std::vector<std::string> fonts;
    DIR *dir;
    struct dirent *ent;
    std::string fontPath = "assets/fonts";
    
    dir = opendir(fontPath.c_str());
    if (dir != nullptr) {
        while ((ent = readdir(dir)) != nullptr) {
            std::string filename = ent->d_name;
            if (filename.length() > 4 && 
                (filename.substr(filename.length() - 4) == ".ttf" || 
                 filename.substr(filename.length() - 4) == ".otf")) {
                fonts.push_back(filename);
            }
        }
        closedir(dir);
    }
    
    return fonts;
}

struct ClockSettings {
    int timeFormat = 24;
    bool showSeconds = true;
    bool showTimezone = true;
    SDL_Color backgroundColor = {0, 0, 0, 255};
    SDL_Color fontColor = {255, 255, 255, 255};
    std::vector<SDL_Color> colors;
    std::vector<std::string> timezones = {"UTC", "America/New_York", "Europe/London", "Asia/Tokyo"};
    std::string selectedTimezone = "UTC";
    std::string uiScale = "medium"; // New UI scaling option
    std::string selectedFont = "DejaVuSans.ttf"; // Default font
    std::vector<std::string> availableFonts = get_available_fonts(); // Dynamically get fonts
};

void save_settings(const std::string &path, const ClockSettings &settings) {
    std::ofstream file(path);
    file << "[Clock]\n";
    file << "timeFormat=" << settings.timeFormat << "\n";
    file << "showSeconds=" << (settings.showSeconds ? "1" : "0") << "\n";
    file << "showTimezone=" << (settings.showTimezone ? "1" : "0") << "\n";
    file << "backgroundColor=#" << to_hex(settings.backgroundColor) << "\n";
    file << "fontColor=#" << to_hex(settings.fontColor) << "\n";
    file << "colors=";
    for (size_t i = 0; i < settings.colors.size(); ++i) {
        if (i > 0) file << ",";
        file << "#" << to_hex(settings.colors[i]);
    }
    file << "\n";
    file << "timezones=";
    for (size_t i = 0; i < settings.timezones.size(); ++i) {
        if (i > 0) file << ",";
        file << settings.timezones[i];
    }
    file << "\n";
    file << "selectedTimezone=" << settings.selectedTimezone << "\n";
    file << "uiScale=" << settings.uiScale << "\n";
    file << "selectedFont=" << settings.selectedFont << "\n";
}

ClockSettings load_settings(const std::string &path) {
    ClockSettings settings;
    // Update available fonts before loading settings
    settings.availableFonts = get_available_fonts();
    std::ifstream file(path);
    std::string line;
    while (std::getline(file, line)) {
        auto eq = line.find('=');
        if (eq == std::string::npos) continue;
        std::string key = line.substr(0, eq);
        std::string val = line.substr(eq + 1);
        if (key == "timeFormat") settings.timeFormat = std::stoi(val);
        else if (key == "showSeconds") settings.showSeconds = (val == "1" || val == "true");
        else if (key == "showTimezone") settings.showTimezone = (val == "1" || val == "true");
        else if (key == "backgroundColor") settings.backgroundColor = from_hex(val);
        else if (key == "fontColor") settings.fontColor = from_hex(val);
        else if (key == "colors") {
            std::stringstream ss(val);
            std::string hex;
            while (std::getline(ss, hex, ',')) {
                settings.colors.push_back(from_hex(hex));
            }
        }
        else if (key == "timezones") {
            settings.timezones.clear(); // Clear existing timezones before loading
            std::stringstream ss(val);
            std::string tz;
            while (std::getline(ss, tz, ',')) {
                settings.timezones.push_back(tz);
            }
        }
        else if (key == "selectedTimezone") settings.selectedTimezone = val;
        else if (key == "uiScale") {
            // Only allow specific scale values
            if (val == "small" || val == "medium" || val == "large")
                settings.uiScale = val;
        }
        else if (key == "selectedFont") {
            // Check if the font is in the available fonts list
            if (std::find(settings.availableFonts.begin(), settings.availableFonts.end(), val) != settings.availableFonts.end()) {
                settings.selectedFont = val;
            }
        }
    }
    if (settings.colors.empty())
        settings.colors = {{0, 0, 0, 255}, {255, 255, 255, 255}, {255, 0, 0, 255}};
    return settings;
}


int main() {
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    SDL_DisplayMode dm;
    SDL_GetCurrentDisplayMode(0, &dm);
    SDL_Window *window = SDL_CreateWindow("Clock", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, dm.w, dm.h, SDL_WINDOW_FULLSCREEN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    ClockSettings settings = load_settings("settings.ini");

    // Dynamic font sizing based on UI scale
    int largeFontSize = settings.uiScale == "small" ? 50 : (settings.uiScale == "medium" ? 100 : 150);
    int smallFontSize = settings.uiScale == "small" ? 16 : (settings.uiScale == "medium" ? 32 : 48);
    std::string fontPath = "assets/fonts/" + settings.selectedFont;
    TTF_Font *fontLarge = TTF_OpenFont(fontPath.c_str(), largeFontSize);
    TTF_Font *fontSmall = TTF_OpenFont(fontPath.c_str(), smallFontSize);
    std::string lastUiScale = settings.uiScale;

    bool showMenu = false;
    int selectedMenuItem = 0;
    std::vector<std::string> menuItems = {"Time Format", "Background Color", "Font Color", "Show Seconds", "Time Zone", "Show Timezone", "UI Scale", "Font Selection"};

    bool running = true;
    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT || (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE))
                running = false;
            else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_m)
                showMenu = !showMenu;
            else if (showMenu && e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_DOWN:
                        selectedMenuItem = (selectedMenuItem + 1) % menuItems.size();
                        break;
                    case SDLK_UP:
                        selectedMenuItem = (selectedMenuItem - 1 + menuItems.size()) % menuItems.size();
                        break;
                    case SDLK_LEFT:
                    case SDLK_RIGHT: {
                        bool right = (e.key.keysym.sym == SDLK_RIGHT);
                        auto &col = settings.colors;
                        if (menuItems[selectedMenuItem] == "Time Format") {
                            settings.timeFormat = settings.timeFormat == 12 ? 24 : 12;
                        } else if (menuItems[selectedMenuItem] == "Show Seconds") {
                            settings.showSeconds = !settings.showSeconds;
                        } else if (menuItems[selectedMenuItem] == "Background Color") {
                            auto it = std::find_if(col.begin(), col.end(), [&](auto c) { return SDLColorEqual{}(c, settings.backgroundColor); });
                            int index = std::distance(col.begin(), it);
                            settings.backgroundColor = col[(index + (right ? 1 : col.size() - 1)) % col.size()];
                        } else if (menuItems[selectedMenuItem] == "Font Color") {
                            auto it = std::find_if(col.begin(), col.end(), [&](auto c) { return SDLColorEqual{}(c, settings.fontColor); });
                            int index = std::distance(col.begin(), it);
                            settings.fontColor = col[(index + (right ? 1 : col.size() - 1)) % col.size()];
                        } else if (menuItems[selectedMenuItem] == "Time Zone") {
                            int index = std::find(settings.timezones.begin(), settings.timezones.end(), settings.selectedTimezone) - settings.timezones.begin();
                            settings.selectedTimezone = settings.timezones[(index + (right ? 1 : settings.timezones.size() - 1)) % settings.timezones.size()];
                        } else if (menuItems[selectedMenuItem] == "Show Timezone") {
                            settings.showTimezone = !settings.showTimezone;
                        } else if (menuItems[selectedMenuItem] == "UI Scale") {
                            // Define the scale options in order
                            const std::vector<std::string> scaleOptions = {"small", "medium", "large"};
                            
                            // Find current scale in the options
                            auto it = std::find(scaleOptions.begin(), scaleOptions.end(), settings.uiScale);
                            size_t index = 0;
                            
                            if (it != scaleOptions.end()) {
                                index = std::distance(scaleOptions.begin(), it);
                                if (right) {
                                    // Move to next scale (wrap around if at the end)
                                    index = (index + 1) % scaleOptions.size();
                                } else {
                                    // Move to previous scale (wrap around if at the beginning)
                                    index = (index - 1 + scaleOptions.size()) % scaleOptions.size();
                                }
                            }
                            
                            // Update the scale
                            settings.uiScale = scaleOptions[index];
                            
                            // Recalculate font sizes based on new scale
                            int largeFontSize = scaleOptions[index] == "small" ? 50 : (scaleOptions[index] == "medium" ? 100 : 150);
                            int smallFontSize = scaleOptions[index] == "small" ? 16 : (scaleOptions[index] == "medium" ? 32 : 48);
                            
                            // Reload fonts with new sizes
                            TTF_CloseFont(fontLarge);
                            TTF_CloseFont(fontSmall);
                            std::string fontPath = "assets/fonts/" + settings.selectedFont;
                            fontLarge = TTF_OpenFont(fontPath.c_str(), largeFontSize);
                            fontSmall = TTF_OpenFont(fontPath.c_str(), smallFontSize);
                        } else if (menuItems[selectedMenuItem] == "Font Selection") {
                            // Dynamically update available fonts
                            settings.availableFonts = get_available_fonts();
                            
                            // Cycle through available fonts
                            if (!settings.availableFonts.empty()) {
                                auto it = std::find(settings.availableFonts.begin(), settings.availableFonts.end(), settings.selectedFont);
                                size_t index = 0;
                                
                                if (it != settings.availableFonts.end()) {
                                    index = std::distance(settings.availableFonts.begin(), it);
                                    if (right) {
                                        // Move to next font (wrap around if at the end)
                                        index = (index + 1) % settings.availableFonts.size();
                                    } else {
                                        // Move to previous font (wrap around if at the beginning)
                                        index = (index - 1 + settings.availableFonts.size()) % settings.availableFonts.size();
                                    }
                                } else {
                                    // Current font not found, default to first font
                                    index = 0;
                                }
                                settings.selectedFont = settings.availableFonts[index];
                            }
                            
                            // Recalculate font sizes based on current scale
                            int largeFontSize = settings.uiScale == "small" ? 50 : (settings.uiScale == "medium" ? 100 : 150);
                            int smallFontSize = settings.uiScale == "small" ? 16 : (settings.uiScale == "medium" ? 32 : 48);
                            
                            // Immediately reload fonts
                            TTF_CloseFont(fontLarge);
                            TTF_CloseFont(fontSmall);
                            std::string fontPath = "assets/fonts/" + settings.selectedFont;
                            fontLarge = TTF_OpenFont(fontPath.c_str(), largeFontSize);
                            fontSmall = TTF_OpenFont(fontPath.c_str(), smallFontSize);
                        }
                        save_settings("settings.ini", settings);
                        break;
                    }
                }
            } else if (!showMenu && e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_LEFT) {
                    int index = std::find(settings.timezones.begin(), settings.timezones.end(), settings.selectedTimezone) - settings.timezones.begin();
                    settings.selectedTimezone = settings.timezones[(index - 1 + settings.timezones.size()) % settings.timezones.size()];
                    save_settings("settings.ini", settings);
                } else if (e.key.keysym.sym == SDLK_RIGHT) {
                    int index = std::find(settings.timezones.begin(), settings.timezones.end(), settings.selectedTimezone) - settings.timezones.begin();
                    settings.selectedTimezone = settings.timezones[(index + 1) % settings.timezones.size()];
                    save_settings("settings.ini", settings);
                }
            }
        }

        // Dynamically recreate fonts if UI scale changes
        if (settings.uiScale != lastUiScale) {
            TTF_CloseFont(fontLarge);
            TTF_CloseFont(fontSmall);
            
            int largeFontSize = settings.uiScale == "small" ? 50 : (settings.uiScale == "medium" ? 100 : 150);
            int smallFontSize = settings.uiScale == "small" ? 16 : (settings.uiScale == "medium" ? 32 : 48);
            
            std::string fontPath = "assets/fonts/" + settings.selectedFont;
            fontLarge = TTF_OpenFont(fontPath.c_str(), largeFontSize);
            fontSmall = TTF_OpenFont(fontPath.c_str(), smallFontSize);
            
            lastUiScale = settings.uiScale;
        }

        SDL_SetRenderDrawColor(renderer, settings.backgroundColor.r, settings.backgroundColor.g, settings.backgroundColor.b, 255);
        SDL_RenderClear(renderer);

        if (!showMenu) {
            std::string time = get_time_in_zone(settings.selectedTimezone, settings.timeFormat, settings.showSeconds);
            SDL_Surface *surf = TTF_RenderText_Blended(fontLarge, time.c_str(), settings.fontColor);
            SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surf);
            int tw, th;
            SDL_QueryTexture(tex, nullptr, nullptr, &tw, &th);
            SDL_Rect dst = {(dm.w - tw) / 2, (dm.h - th) / 2, tw, th};
            SDL_RenderCopy(renderer, tex, nullptr, &dst);
            SDL_FreeSurface(surf);
            SDL_DestroyTexture(tex);

            if (settings.showTimezone) {
                std::string tz = settings.selectedTimezone;
                SDL_Surface *stz = TTF_RenderText_Blended(fontSmall, tz.c_str(), settings.fontColor);
                SDL_Texture *ttz = SDL_CreateTextureFromSurface(renderer, stz);
                int sw, sh;
                SDL_QueryTexture(ttz, nullptr, nullptr, &sw, &sh);
                SDL_Rect dsttz = {(dm.w - sw) / 2, dst.y + th + 40, sw, sh};
                SDL_RenderCopy(renderer, ttz, nullptr, &dsttz);
                SDL_FreeSurface(stz);
                SDL_DestroyTexture(ttz);
            }
        } else {
            // Find the maximum width of menu item titles
            int maxTitleWidth = 0;
            std::vector<SDL_Surface*> titleSurfaces;
            std::vector<SDL_Texture*> titleTextures;
            std::vector<int> titleWidths;
            std::vector<int> titleHeights;
            
            for (size_t i = 0; i < menuItems.size(); ++i) {
                std::string menuItemTitle = (selectedMenuItem == (int)i ? "* " : "  ") + menuItems[i];
                SDL_Surface *titleSurf = TTF_RenderText_Blended(fontSmall, menuItemTitle.c_str(), settings.fontColor);
                SDL_Texture *titleTex = SDL_CreateTextureFromSurface(renderer, titleSurf);
                int titleW, titleH;
                SDL_QueryTexture(titleTex, nullptr, nullptr, &titleW, &titleH);
                
                maxTitleWidth = std::max(maxTitleWidth, titleW);
                titleSurfaces.push_back(titleSurf);
                titleTextures.push_back(titleTex);
                titleWidths.push_back(titleW);
                titleHeights.push_back(titleH);
            }
            
            // Render titles and values with consistent alignment
            for (size_t i = 0; i < menuItems.size(); ++i) {
                std::string value;
                if (menuItems[i] == "Time Format") value = std::to_string(settings.timeFormat);
                else if (menuItems[i] == "Show Seconds") value = settings.showSeconds ? "Yes" : "No";
                else if (menuItems[i] == "Time Zone") value = settings.selectedTimezone;
                else if (menuItems[i] == "Show Timezone") value = settings.showTimezone ? "Yes" : "No";
                else if (menuItems[i] == "UI Scale") value = settings.uiScale;
                else if (menuItems[i] == "Background Color") value = "#" + to_hex(settings.backgroundColor);
                else if (menuItems[i] == "Font Color") value = "#" + to_hex(settings.fontColor);
                else if (menuItems[i] == "Font Selection") {
                    value = settings.selectedFont;
                }

                // Dynamically adjust vertical spacing based on UI scale
                int verticalSpacing = settings.uiScale == "small" ? 40 : (settings.uiScale == "medium" ? 50 : 60);
                
                // Render menu item title
                std::string menuItemTitle = (selectedMenuItem == (int)i ? "* " : "  ") + menuItems[i];
                SDL_Surface *titleSurf = titleSurfaces[i];
                SDL_Texture *titleTex = titleTextures[i];
                int titleW = titleWidths[i];
                int titleH = titleHeights[i];
                
                // Position menu items in the top-left of the screen
                SDL_Rect titleRect = {50, static_cast<int>(50 + i * verticalSpacing), titleW, titleH};
                SDL_RenderCopy(renderer, titleTex, nullptr, &titleRect);
                
                // Render menu item value
                SDL_Surface *valueSurf = TTF_RenderText_Blended(fontSmall, value.c_str(), settings.fontColor);
                SDL_Texture *valueTex = SDL_CreateTextureFromSurface(renderer, valueSurf);
                int valueW, valueH;
                SDL_QueryTexture(valueTex, nullptr, nullptr, &valueW, &valueH);
                SDL_Rect valueRect = {titleRect.x + maxTitleWidth + 30, titleRect.y, valueW, valueH};
                SDL_RenderCopy(renderer, valueTex, nullptr, &valueRect);
                
                // Clean up surfaces and textures
                SDL_FreeSurface(titleSurf);
                SDL_DestroyTexture(titleTex);
                SDL_FreeSurface(valueSurf);
                SDL_DestroyTexture(valueTex);
            }
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(1000 / 60);
    }

    TTF_CloseFont(fontSmall);
    TTF_CloseFont(fontLarge);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}