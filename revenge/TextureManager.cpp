#include "TextureManager.h"
#include <filesystem> // Required for getAvailableGraphicsPacks

// Initialize static members
std::map<std::string, SDL_Texture*> TextureManager::s_textureMap;
std::map<std::string, std::string> TextureManager::s_textureFileNames;
std::string TextureManager::s_currentGraphicsPackPath = "assets/images/default/"; // Default pack
std::string TextureManager::s_currentGraphicsPackName = "default"; // Default pack name

// Register a texture ID with its corresponding filename
void TextureManager::registerTexture(const std::string& id, const std::string& fileName) {
    s_textureFileNames[id] = fileName;
    //std::cout << "TextureManager: Registered texture ID '" << id << "' with filename '" << fileName << "'" << std::endl;
}

// Load a texture by its ID using the current graphics pack
bool TextureManager::loadTexture(const std::string& id, SDL_Renderer* renderer) {
    auto it = s_textureFileNames.find(id);
    if (it == s_textureFileNames.end()) {
        std::cerr << "TextureManager: Attempting to load unregistered texture ID '" << id << "'. Register it first." << std::endl;
        return false;
    }
    const std::string& fileName = it->second;
    std::string fullPath = s_currentGraphicsPackPath + fileName;

    //std::cout << "TextureManager: Attempting to load texture '" << id << "' from: " << fullPath << std::endl;

    SDL_Surface* tempSurface = IMG_Load(fullPath.c_str());

    if (tempSurface == nullptr) {
        //std::cout << "TextureManager: Failed to load texture '" << id << "' from current pack path: " << fullPath << ". Error: " << IMG_GetError() << std::endl;
        if (s_currentGraphicsPackName != "default") {
            std::string fallbackPath = "assets/images/default/" + fileName;
            //std::cout << "TextureManager: Attempting fallback for '" << id << "' to: " << fallbackPath << std::endl;
            tempSurface = IMG_Load(fallbackPath.c_str());
            if (tempSurface == nullptr) {
                std::cerr << "TextureManager: Failed to load texture '" << id << "' (file: " << fileName << ") from current pack '" << s_currentGraphicsPackName << "' (path: " << fullPath << ") AND from default pack (path: " << fallbackPath << "). Error: " << IMG_GetError() << std::endl;
                return false;
            } else {
                //std::cout << "TextureManager: Successfully loaded '" << id << "' from default pack via fallback." << std::endl;
            }
        } else {
            // Current pack is 'default' and it failed to load
            std::cerr << "TextureManager: Failed to load texture '" << id << "' (file: " << fileName << ") from default pack path: " << fullPath << ". Error: " << IMG_GetError() << std::endl;
            return false;
        }
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, tempSurface);
    SDL_FreeSurface(tempSurface);

    if (texture != nullptr) {
        if (s_textureMap.count(id)) {
            SDL_DestroyTexture(s_textureMap[id]);
        }
        s_textureMap[id] = texture;
        //std::cout << "TextureManager: Successfully loaded texture '" << id << "' into map." << std::endl;
        return true;
    }

    std::cerr << "TextureManager: Failed to create texture from surface for '" << id << "' (file: " << fileName << "). Error: " << SDL_GetError() << std::endl;
    return false;
}

// Set the current graphics pack and reload all registered textures
bool TextureManager::setGraphicsPack(const std::string& packName, SDL_Renderer* renderer) {
    std::string newPackPath = "assets/images/" + packName + "/";
    //std::cout << "TextureManager: Attempting to set graphics pack to '" << packName << "' at path '" << newPackPath << "'" << std::endl;

    // Check if the directory exists
    if (!std::filesystem::is_directory(newPackPath)) {
        std::cerr << "TextureManager: Graphics pack directory not found: " << newPackPath << std::endl;
        return false;
    }

    s_currentGraphicsPackName = packName;
    s_currentGraphicsPackPath = newPackPath;

    clear(); // Clear existing textures from the map (but not from s_textureFileNames)

    bool allLoaded = true;
    for (const auto& pair : s_textureFileNames) {
        if (!loadTexture(pair.first, renderer)) {
            std::cerr << "TextureManager: Failed to load texture '" << pair.first << "' from new pack '" << packName << "'." << std::endl;
            allLoaded = false; // Continue loading others, but report failure
        }
    }
    //std::cout << "TextureManager: Finished setting graphics pack to '" << packName << "'. All textures loaded: " << (allLoaded ? "Yes" : "No") << std::endl;
    return allLoaded; // Or true if partial success is acceptable and errors are handled elsewhere
}

// Get a list of available graphics packs by scanning the assets/images directory
std::vector<std::string> TextureManager::getAvailableGraphicsPacks() {
    std::vector<std::string> packs;
    const std::string basePath = "assets/images/";
    try {
        for (const auto& entry : std::filesystem::directory_iterator(basePath)) {
            if (entry.is_directory()) {
                packs.push_back(entry.path().filename().string());
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "TextureManager: Error accessing graphics pack directory '" << basePath << "': " << e.what() << std::endl;
    }
    //std::cout << "TextureManager: Found " << packs.size() << " available graphics packs." << std::endl;
    return packs;
}

// Get the name of the currently active graphics pack
std::string TextureManager::getCurrentGraphicsPackName() {
    return s_currentGraphicsPackName;
}

void TextureManager::draw(const std::string& id, int x, int y, int width, int height, SDL_Renderer* renderer, int offsetX, int offsetY, SDL_RendererFlip flip) {
    if (s_textureMap.find(id) == s_textureMap.end()) {
        std::cerr << "Attempting to draw non-existent texture with id: " << id << std::endl;
        return;
    }
    SDL_Rect srcRect = {0, 0, width, height};
    SDL_Rect destRect = {x + offsetX, y + offsetY, width, height};
    SDL_RenderCopyEx(renderer, s_textureMap[id], &srcRect, &destRect, 0, 0, flip);
}

void TextureManager::drawFrame(const std::string& id, int x, int y, int width, int height, int currentRow, int currentFrame, SDL_Renderer* renderer, SDL_RendererFlip flip) {
    if (s_textureMap.find(id) == s_textureMap.end()) {
        std::cerr << "Attempting to drawFrame for non-existent texture with id: " << id << std::endl;
        return;
    }
    SDL_Rect srcRect = {width * currentFrame, height * (currentRow - 1), width, height};
    SDL_Rect destRect = {x, y, width, height};
    SDL_RenderCopyEx(renderer, s_textureMap[id], &srcRect, &destRect, 0, 0, flip);
}

void TextureManager::clearTexture(const std::string& id) {
    auto it = s_textureMap.find(id);
    if (it != s_textureMap.end()) {
        SDL_DestroyTexture(it->second);
        s_textureMap.erase(it);
    } else {
        std::cerr << "Attempting to clear non-existent texture with id: " << id << std::endl;
    }
}

void TextureManager::clear() {
    for (auto const& [id, texture] : s_textureMap) {
        SDL_DestroyTexture(texture);
    }
    s_textureMap.clear();
    // Note: s_textureFileNames is NOT cleared here, as it holds the registry of all known textures.
}
