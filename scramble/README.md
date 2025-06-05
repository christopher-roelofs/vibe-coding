# Word Scramble Game

A C++ and SDL-based word scramble game designed for Linux handhelds with D-pad and face button controls.

## Target Platform
- Linux-based handhelds
- 640x480 screen resolution
- D-pad and face button input

## Dependencies
- SDL2
- SDL2_ttf (for text rendering)

### Installation (Debian/Ubuntu based systems)
```bash
sudo apt-get update
sudo apt-get install libsdl2-dev libsdl2-ttf-dev
```

**Note on Fonts:** This game requires a TrueType Font (`.ttf`) file for displaying text. Place a font file (e.g., `Arial.ttf`) in the `scramble` directory, or modify the `fontPath` variable in `main.cpp` to point to a system font (e.g., `/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf`).

## How to Build
Navigate to the `scramble` directory in your terminal and run:
```bash
make
```

## How to Run
After building, execute the game from the `scramble` directory:
```bash
./word_scramble
```

## Controls (Planned)
- **D-Pad**: Navigate letters/options
- **Face Button A (e.g., Cross/A)**: Select letter / Confirm
- **Face Button B (e.g., Circle/B)**: Deselect letter / Back
- **Start Button**: Pause game / Menu
