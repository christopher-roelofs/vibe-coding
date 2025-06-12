# Word Scramble Game

A simple word scramble game built with C++ and SDL2.

## Prerequisites

- C++ Compiler (g++ recommended, supporting C++17)
- Make
- SDL2 library (Core and Development files)
- SDL2_ttf library (Core and Development files)

### Installation (macOS with Homebrew)
```bash
brew install sdl2 sdl2_ttf
```

### Installation (Debian/Ubuntu)
```bash
sudo apt-get update
sudo apt-get install libsdl2-dev libsdl2-ttf-dev
```

## Setup

1.  **Font File**: 
    Place a TrueType Font (`.ttf`) file (e.g., `VCR_OSD_MONO.ttf` or any other monospaced font) into the `assets/fonts/` directory.
    Ensure the `path` key under the `[font]` section in `settings.ini` correctly points to this font file (e.g., `path=assets/fonts/VCR_OSD_MONO.ttf`).

2.  **Word Lists**:
    Word lists are JSON files located in the `assets/word_lists/` directory. Each file should have:
    - `name`: (string) Display name of the list.
    - `author`: (string) Author of the list.
    - `date`: (string) Creation date.
    - `description`: (string) A short description.
    - `words`: (array of strings) The words for the game (preferably uppercase).
    See `assets/word_lists/animals.json` for an example.

## How to Compile and Run

1.  **Navigate to the project directory**:
    ```bash
    cd /path/to/word_scramble
    ```

2.  **Compile**:
    ```bash
    make
    ```
    If `sdl2-config` is not found or there are issues with SDL paths, you might need to adjust `SDL_CFLAGS`, `SDL_LIBS`, and `SDL_TTF_CFLAGS` in the `Makefile` to point to your SDL2 installation directories.

3.  **Run**:
    ```bash
    make run
    ```
    Or directly:
    ```bash
    ./word_scramble
    ```

## Controls

-   **Arrow Keys (Left/Right)**: Navigate options or characters.
-   **Enter**: Select an option or character.
-   **Backspace/Delete**: Remove the last guessed character.
-   **Spacebar**: Submit a guess (when the guessed word is full).
-   **Escape**: (To be implemented for quitting/pausing)

## Settings

Game settings (colors, font) are configured in `settings.ini`.
-   Colors are in HEX format (e.g., `#RRGGBB`).
-   Font path and size are also set here.
