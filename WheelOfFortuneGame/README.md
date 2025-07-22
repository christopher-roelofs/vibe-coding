# Wheel of Fortune Game

A classic word puzzle game built with C++ and SDL2, where players guess letters to reveal hidden phrases.

## Features

- **Classic Wheel of Fortune gameplay** - Guess letters to reveal hidden phrases
- **4x14 Grid Layout** - Professional game show style with 4 rows of 14 letter boxes  
- **Dual game modes** - Letter guessing mode and puzzle solving mode
- **Interactive alphabet** - Visual alphabet grid with used letter tracking
- **Smart navigation** - Automatically skips solved letters in puzzle mode
- **Multiple categories** - Phrases, Movies, Food, Places, People, and Things
- **Smart text fitting** - Automatically breaks phrases across rows optimally
- **Advanced scoring** - Letter guessing (+100/-50), solving bonus (+2000), completion (+1000)
- **Clean interface** - Simple, easy-to-use visual design
- **Logical scaling** - 640x480 base resolution that scales to any window size
- **INI-based puzzles** - Organized puzzle categories in clean INI format

## Controls

### Letter Guessing Mode (Default)
- **Arrow Keys** - Navigate alphabet grid (2 rows of 13 letters)
- **Letter Keys (A-Z)** - Direct letter input  
- **Enter** - Guess selected letter
- **S** - Toggle to puzzle solving mode
- **A** - Abandon puzzle and reveal answer

### Puzzle Solving Mode
- **Left/Right Arrow Keys** - Move between empty letter boxes
- **Up/Down Arrow Keys** - Cycle through letters A-Z
- **Enter** - Submit puzzle solution attempt
- **S** - Toggle back to letter guessing mode

### General Controls
- **Escape** - Return to start screen
- **Space** - Start game from main menu

## How to Play

### Letter Guessing Phase
1. Run the game and press Enter or Space to start
2. A random phrase will be selected from one of six categories  
3. The phrase is automatically fitted into a 4x14 grid (56 letter boxes total)
4. Navigate the alphabet grid below with arrow keys or type letters directly
5. Press Enter to guess the highlighted letter
6. Correctly guessed letters are revealed and marked as used (dark)
7. Build up revealed letters to help solve the puzzle

### Puzzle Solving Phase  
1. Press **S** to toggle to puzzle solving mode
2. You'll see only previously guessed letters + your manual entries
3. Use Left/Right to move between unsolved letter positions
4. Use Up/Down to cycle through letters A-Z for the current position
5. Fill in ALL empty letter boxes with your best guesses
6. Press **Enter** when complete to submit your solution
7. Win with +2000 bonus points for solving, or +1000 for guessing all letters!

### Giving Up
- Press **A** at any time to abandon the puzzle and reveal the complete answer
- No points are awarded for abandoned puzzles

## Building

### Prerequisites

Make sure you have SDL2 and SDL2_ttf installed:

```bash
# Ubuntu/Debian
sudo apt-get install libsdl2-dev libsdl2-ttf-dev

# Or use the provided install target
make install
```

### Compilation

```bash
make
```

### Running

```bash
./bin/wheeloffortune
```

## Project Structure

```
WheelOfFortuneGame/
├── src/           # Source files
├── include/       # Header files
├── data/          # Game data (puzzles.txt)
├── bin/           # Compiled executable
├── obj/           # Object files
├── Makefile       # Build configuration
└── README.md      # This file
```

## Adding New Puzzles

Edit `data/puzzles.ini` to add new puzzles. INI format with category headers:
```ini
[MOVIE]
THE MATRIX
STAR WARS
JURASSIC PARK

[FOOD]
CHOCOLATE CAKE
PIZZA
HAMBURGER

[PHRASE]
BREAK A LEG
PIECE OF CAKE
```

## Technical Details

- **Language**: C++14
- **Graphics**: SDL2 with SDL2_ttf
- **Architecture**: Object-oriented design with state management
- **Resolution**: 640x480 logical resolution with automatic scaling
- **Platform**: Cross-platform (Linux, macOS, Windows)

## Game Classes

- **Game** - Main game loop, SDL initialization, state management
- **StartScreen** - Main menu interface
- **WheelOfFortune** - Core game logic, rendering, input handling

## Current Status

✅ Core game mechanics implemented  
✅ Keyboard navigation system  
✅ File-based puzzle loading  
✅ Basic scoring system  
✅ Win/lose conditions  
✅ Clean visual interface  

## Future Enhancements

- Sound effects and background music
- Animations for letter reveals
- High score persistence
- Multiple difficulty levels
- Timed challenges
- Hint system
- Better graphics and themes