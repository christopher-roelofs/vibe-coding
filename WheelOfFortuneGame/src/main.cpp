#include "Game.h"
#include <iostream>

int main() {
    Game game;
    
    if (!game.Initialize()) {
        std::cerr << "Failed to initialize game" << std::endl;
        return 1;
    }
    
    game.Run();
    
    return 0;
}