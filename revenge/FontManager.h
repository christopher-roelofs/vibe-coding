#ifndef FONTMANAGER_H
#define FONTMANAGER_H

#include <SDL2/SDL_ttf.h>
#include <string>
#include <map>

class FontManager {
public:
    static bool init();
    static void loadFont(const std::string& id, const std::string& filename, int size);
    static TTF_Font* getFont(const std::string& id); 
    static void drawText(SDL_Renderer* renderer, const std::string& text, int x, int y, const std::string& fontId, SDL_Color color);
    static void quit();

private:
    static std::map<std::string, TTF_Font*> s_fonts;
};

#endif // FONTMANAGER_H
