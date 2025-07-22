# 🎲 Dice Merge

A strategic grid-based puzzle game where you merge dice to create higher values and achieve high scores.

## 🎮 Game Overview

Dice Merge is played on a **6×4 grid** where you select and merge connected dice of the same value to create higher-value dice. The goal is to achieve the highest score possible by creating strategic merges and utilizing special dice effects.

## 🎯 Core Mechanics

### Basic Gameplay
- **Grid Size**: 6 columns × 4 rows (24 total spaces)
- **Controls**: Arrow keys to move cursor, Enter to select/deselect dice, Escape to clear selection
- **Merge Requirement**: Select exactly 3 connected dice (adjacent orthogonally or diagonally)
- **Merge Result**: Creates one die with value +1 at your cursor position, other selected dice disappear

### Dice Values
- **Regular Dice**: Values 1-6 with color coding:
  - 1 = Red, 2 = Blue, 3 = Green, 4 = Yellow, 5 = Purple, 6 = Black
- **Spawn System**: New dice spawn as 1 (50%), 2 (35%), or 3 (15%)
- **6-Dice Rule**: Merging three 6s creates an explosion, clearing the dice and awarding double points

### Physics System
- **Gravity**: After merges, dice fall down to fill empty spaces
- **Refill**: Empty spaces are filled from the top with new random dice
- **Merge Counters**: Dice show "x2", "x3", etc. indicating how many times they've been created through merging

## ⭐ Special Dice System

Special dice spawn with a **5% chance** instead of regular dice and can be merged with any group of connected regular dice as wildcards.

### 💖 Beneficial Special Dice (High Spawn Rate)
- **❤️ Heart**: Removes the lowest value die from the board
- **➕ Plus**: Awards +50 bonus points
- **✖️ Multiply**: Doubles the earned score from the merge
- **💲 Dollar**: Awards +100 bonus points  
- **🛡️ Shield**: Converts all 1s on the board to 2s (+ 10 points per conversion)

### ⚔️ Utility Special Dice (Medium Spawn Rate)
- **👊 Fist**: Smashes all dice in a 3×3 area around a random position
- **📜 Scroll**: Copies the highest value die to an empty space
- **➖ Minus**: Reduces all dice values by 1 (minimum value 1)
- **➗ Divide**: Splits the highest value die in half, creating two dice
- **❓ Question**: Triggers a random effect from any other special die

### 💀 Risky Special Dice (Low Spawn Rate)  
- **💀 Skull**: Removes the highest value die from the board (destructive)
- **⚔️ Sword**: Removes the highest value die + awards bonus points (removed value × 10)

## 🏆 Scoring System

### Base Scoring
- **Merge Score**: `dice_value × 10 × merge_count × merge_history_bonus`
- **Merge History Bonus**: +10% per total merge counter of consumed dice
- **6-Dice Explosion**: Double points when clearing 6s

### Special Dice Bonuses
- **Plus**: +50 points
- **Dollar**: +100 points  
- **Shield**: +10 points per die converted
- **Sword**: +10 points per value of removed die

### High Score
- **Persistent Storage**: High scores are saved to a `highscore` file
- **Auto-Save**: Updates immediately when a new high score is achieved

## 🎮 Strategic Elements

### Selection Rules
- Must select exactly 3 connected dice
- Regular dice must all have the same value
- Special dice act as wildcards and can join any group
- Invalid selections (not connected or <3 dice) auto-deselect
- New merged die appears at cursor position (last selected tile)

### Advanced Strategy
- **Merge Multipliers**: Dice created through multiple merges have higher value multipliers
- **Special Dice Timing**: Use special dice immediately for effects vs. saving for bigger merges  
- **Board Control**: Some special dice (Shield, Minus, Fist) affect the entire board
- **Risk Management**: Skull and Sword can hurt your board but provide unique effects

## 🛠️ Technical Details

### Built With
- **C++11** with SDL2, SDL2_ttf, SDL2_image
- **Visual Assets**: Individual PNG files for each dice type and special dice
- **Window Size**: 640×480 pixels
- **Tile Size**: 90×90 pixels with 10px margins

### File Structure
```
assets/
├── fonts/InterVariable.ttf
└── images/
    ├── 128px/          # Regular dice (6 colors × 6 values)
    └── cute/           # Special dice (0.png - 11.png)
```

### Build & Run
```bash
make clean && make
./dice_merge
```

## 🎯 Game Flow Example

1. **Start**: 6×4 grid fills with random 1s, 2s, 3s, and occasional special dice
2. **Select**: Use arrow keys to move cursor, Enter to select 3 connected dice
3. **Merge**: Selected dice combine into higher value at cursor position  
4. **Effects**: Special dice trigger their effects (points, board modifications, etc.)
5. **Physics**: Dice fall down, empty spaces refill from top
6. **Repeat**: Continue merging to build higher values and score points

The game continues indefinitely with the goal of achieving the highest possible score through strategic merging and special dice utilization.

## 🙏 Credits

### Art Assets
- **Special Dice Icons**: [Dice Anim by Dani Maccari](https://dani-maccari.itch.io/dice-anim)
- **Colored Dice Assets**: [Multi Color Dice Assets by Maps and Apps](https://mapsandapps.itch.io/multi-color-dice-assets)

### Development
- **Game Design & Programming**: Christopher Roelofs
- **Engine**: SDL2, SDL2_ttf, SDL2_image