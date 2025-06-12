#include "Game.h"
#include <iostream>

int main(int argc, char* argv[]) {
    // Suppress unused parameter warnings for argc and argv if not used
    (void)argc;
    (void)argv;

    Game game;
    if (!game.init()) {
        std::cerr << "Game initialization failed!" << std::endl;
        return 1;
    }

    game.run();

    return 0;
}
