# Open Revenge

A C++ and SDL2 clone of the classic game Rodent's Revenge. This version features a flexible level editor, custom graphics packs, and persistent settings.

The goal is to guide the mouse (`M`) to trap all the cats (`K`) on the level by pushing blocks (`B`). Once all cats are trapped, you can safely collect the cheese (`C`) to win.

## [Level Editor](https://christopher-roelofs.github.io/vibe-coding/level_editor.html) (`level_editor.html`)

An in-browser tool to create and modify level packs. Just open the file in a modern web browser.

## Prerequisites

- C++ Compiler (supporting C++17, e.g., g++ or Clang)
- Make
- SDL2
- SDL2_image
- SDL2_ttf

### macOS Installation (using Homebrew)

```bash
brew install sdl2 sdl2_image sdl2_ttf
# Make and a C++ compiler (like Clang) are typically pre-installed or come with Xcode Command Line Tools.
```

### Linux Installation (e.g., Ubuntu/Debian)

```bash
sudo apt-get update
sudo apt-get install build-essential libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev
# 'build-essential' usually includes make and g++.
```

## Build Instructions

1. Navigate to the project directory:

   ```bash
   cd /path/to/vibes/revenge
   ```
2. Compile the project using Make:

   ```bash
   make
   ```

   The executable will be created in the project root directory (e.g., `revenge`).

## Running the Game

After compiling, run the executable from the **root project directory** (`revenge/`) to ensure it can find the `assets` folder.

```bash
./revenge
# Or if your Makefile places it in a build subfolder, adjust accordingly (e.g., ./build/revenge)
```

## Game Controls

- **Arrow Keys**: Move the mouse.
- **S**: Open or close the Settings menu (change graphics pack).
- **Escape**: Exit the game from the Level Select screen.
- N: Next level.
- P: Previous level.

## Directory Structure

- `assets/`: Contains all game assets.
  - `fonts/`: Font files (`.ttf`) for rendering text.
  - `images/`: Contains graphics packs, each in its own subdirectory (e.g., `default/`).
  - `levels/`: Contains level pack files (`.lvl`).
- `level_editor.html`: A powerful, browser-based tool to create and edit level packs.
- `settings.txt`: An auto-generated file that stores user settings, like the last selected graphics pack.

### Features

- **Load & Save:** Load `.lvl` files from your computer, or paste raw level text directly. Export your creations as a `.lvl` file.
- **Live Editing:** A visual grid editor and a live text area are synced in real-time. Edit visually or by typing.
- **Pack & Level Management:** Create new packs, add, delete, and duplicate levels within a pack.
- **Grid Tools:** Resize the grid and paint with a palette of all available game tiles.

## Customization

### Graphics Packs

You can create your own graphics by adding a new folder inside `assets/images/`. The game will automatically detect it and make it available in the settings menu. If a custom pack is missing an image, the game will fall back to the `default` pack's image for that tile.

### Settings

The game saves your last-used graphics pack to `settings.txt` and will load it automatically the next time you play.
