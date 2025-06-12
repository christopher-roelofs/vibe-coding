# Word Scramble

A fun and challenging word scramble game built with C++ and the SDL2 library. Test your vocabulary by unscrambling words from various themed lists!

## Features

*   **Multiple Word Lists:** Play with different categories of words (e.g., animals, fruits). Easily add your own custom lists.
*   **Configurable Appearance:** Customize game colors using the `settings.ini` file or the provided `settings_editor.html`.
*   **Simple Controls:** Uses keyboard input for easy navigation and gameplay.
*   **Cross-Platform:** Built with SDL2, making it potentially portable to other operating systems.
*   **[Settings Editor](https://christopher-roelofs.github.io/vibe-coding/settings_editor.html):** A standalone HTML tool (`settings_editor.html`) allows for easy visual editing of color schemes in `settings.ini`.

## Prerequisites

Before you can build and play Word Scramble, you'll need the following installed on your system:

*   **C++ Compiler:** A modern C++ compiler that supports C++17 (e.g., GCC, Clang).
*   **Make:** The build automation tool.
*   **SDL2:** Simple DirectMedia Layer library.
    *   On macOS (using Homebrew): `brew install sdl2`
    *   On Debian/Ubuntu: `sudo apt-get install libsdl2-dev`
*   **SDL2_ttf:** SDL2 TrueType Font rendering library.
    *   On macOS (using Homebrew): `brew install sdl2_ttf`
    *   On Debian/Ubuntu: `sudo apt-get install libsdl2-ttf-dev`
*   **nlohmann/json.hpp:** A single-header JSON library for C++. You will need to download `json.hpp` from [the nlohmann/json GitHub repository](https://github.com/nlohmann/json/tree/develop/single_include/nlohmann) and place it in the `src/` directory.
*   **A TTF Font:** You need to provide a TrueType Font file (e.g., `font.ttf`). Place it in the `assets/fonts/` directory. The game looks for a font specified in `settings.ini` (or uses a default if not specified).

## Building the Game

1.  **Clone the repository (if applicable) or ensure all source files are in place.**
2.  **Navigate to the project's root directory** (the one containing the `Makefile`).
3.  **Ensure `nlohmann/json.hpp` is in the `src/` directory.**
4.  **Ensure your TTF font file is in `assets/fonts/`.**
5.  **Run Make:**
    ```bash
    make
    ```
    This will compile the source files and create an executable named `scramble` in the project's root directory.

## Running the Game

After successful compilation, you can run the game from the project's root directory:

```bash
./scramble
```

## Configuration

Game settings are managed through the `settings.ini` file located in the project's root directory.

### `settings.ini`

This file allows you to configure:

*   **Font:** Specify the filename of the TTF font to use (located in `assets/fonts/`).
    ```ini
    [font]
    font = your_font_name.ttf
    ```
*   **Colors:** Define the color scheme for various game elements (background, text, highlighted text, etc.). Colors are specified in `R,G,B` format.
    ```ini
    [colors]
    background_color = 30,30,30
    text_color = 200,200,200
    highlight_color = 255,255,0
    scrambled_word_color = 220,220,220
    input_text_color = 180,220,180
    feedback_correct_color = 0,255,0
    feedback_incorrect_color = 255,0,0
    list_selection_color = 100,150,255
    list_item_color = 200,200,200
    ```

### `settings_editor.html`

For a more user-friendly way to edit the color scheme, open the `settings_editor.html` file (located in the project root) in a web browser.

*   **Load `settings.ini`:** Click the "Load settings.ini" button and select your `settings.ini` file.
*   **Edit Colors:** Use the color pickers to adjust the colors for different UI elements. A live preview will show your changes.
*   **Save Changes:** Click the "Download Updated settings.ini" button to save your new color scheme. Replace your existing `settings.ini` with this downloaded file.

## Word Lists

Word lists are stored as JSON files in the `assets/word_lists/` directory. Each JSON file should contain a single array of strings.

Example (`animals.json`):

```json
[
  "lion",
  "tiger",
  "bear",
  "elephant",
  "zebra"
]
```

To add a new word list:

1.  Create a new `.json` file in the `assets/word_lists/` directory.
2.  Format it as an array of words (strings).
3.  The game will automatically detect and load any `.json` files from this directory on startup.

## Controls

*   **Start Screen (Word List Selection):**
    *   **Up/Down Arrows:** Navigate through the available word lists.
    *   **Enter:** Select the highlighted word list and start the game.
*   **Game Screen:**
    *   **Alphabet Keys (A-Z):** Type letters to form your guess.
    *   **Backspace:** Delete the last typed letter.
    *   **Enter:** Submit your guess.
*   **General:**
    *   **Escape:** Quit the game from most screens.

## Directory Structure

```
word_scramble/
├── assets/
│   ├── fonts/          # Place your .ttf font files here
│   │   └── (example_font.ttf)
│   └── word_lists/     # Place your .json word list files here
│       ├── animals.json
│       └── fruits.json
├── src/
│   ├── Game.cpp
│   ├── Game.h
│   ├── main.cpp
│   ├── Settings.cpp
│   ├── Settings.h
│   ├── WordList.cpp
│   ├── WordList.h
│   └── (nlohmann/json.hpp) # You need to add this file
├── Makefile
├── README.md           # This file
├── settings.ini        # Game configuration file
├── settings_editor.html # HTML tool for editing color settings
└── word_scramble       # Executable (after compilation)
```

Enjoy the game!

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
