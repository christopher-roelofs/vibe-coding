# Minesweeper (SDL2 C++)

A simple Minesweeper game built with C++ and SDL2, designed for keyboard-only input and 640x480 resolution displays.

## Dependencies

-   SDL2: `sudo apt-get install libsdl2-dev` (Debian/Ubuntu) or `brew install sdl2` (macOS)
-   SDL2_ttf: `sudo apt-get install libsdl2-ttf-dev` (Debian/Ubuntu) or `brew install sdl2_ttf` (macOS)
-   A TTF font file (e.g., `Arial.ttf`). Place it in the same directory as the executable or update the path in `main.cpp`.

## Build Instructions

1.  Make sure you have a C++ compiler (like g++), SDL2, and SDL2_ttf installed.
2.  Place a TTF font file (e.g., `Arial.ttf`) in the project's root directory.
3.  Open your terminal in the project directory.
4.  Run `make` to compile the game.

## How to Play

-   Run the compiled executable: `./minesweeper`
-   **Arrow Keys**: Navigate the grid.
-   **Spacebar / Enter**: Reveal the selected cell.
-   **F Key**: Place or remove a flag on the selected cell.
-   **R Key**: Restart the game.
-   **Escape Key**: Quit the game.

## Game Logic Overview

-   The game board is a grid of cells.
-   Some cells contain mines, others are empty.
-   Revealing an empty cell shows the number of adjacent mines.
-   If an empty cell with no adjacent mines is revealed, all its neighboring empty cells are automatically revealed.
-   The goal is to reveal all non-mine cells.
-   Flag cells you suspect contain mines.
-   If you reveal a mine, you lose.
