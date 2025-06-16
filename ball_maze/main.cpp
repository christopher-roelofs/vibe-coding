#include "Game.h"
#include <iostream>

int main(int argc, char* argv[]) {
    Game game;
    if (!game.init()) {
        std::cerr << "Failed to initialize game." << std::endl;
        return -1;
    }

    // Start the game with the level selection screen
    game.run();

    return 0;
}
