# Ball Maze Game (SDL2 + Box2D 3.x)

A simple 2D ball maze game where the player rotates the maze to guide a ball to a goal.

## Dependencies
- SDL2
- Box2D 3.x

## Building

Ensure SDL2 and Box2D 3.x are installed.
- For SDL2, `brew install sdl2` on macOS is common.
- For Box2D 3.x, it might be installed via a package manager or compiled from source. Ensure headers are typically in `/usr/local/include` and libraries in `/usr/local/lib` or that your compiler can find them.

To build:
```bash
make
```

To run:
```bash
./ball_maze_game
```

To clean:
```bash
make clean
```

## Project Structure

- `main.cpp`: Entry point.
- `Game.h/.cpp`: Main game logic, SDL/Box2D setup, game loop.
- `Level.h/.cpp`: Handles loading maze layouts from text files.
- `Maze.h/.cpp`: Represents the maze, its walls (static Box2D bodies), and rotation.
- `Ball.h/.cpp`: Represents the player-controlled ball (dynamic Box2D body).
- `Warp.h/.cpp`: Implements warp objects that teleport the ball between paired points.
- `constants.h`: Global constants for screen size, physics, etc.
- `Makefile`: Build script.
- `assets/`: Directory for game assets (e.g., level files).

## Level File Format

Levels are defined in text files with the following character mappings:

- `#`: Wall (impassable)
- `.`: Empty space
- `O`: Ball starting position
- `G`: Goal position
- `H`: Hole (resets the level when ball falls in)
- `R`: Reverse control item (inverts controls temporarily)
- `1`: Warp point (teleports ball to another '1' warp point)

### Warp Points

Warp points are defined by the character '1'. When the ball touches a warp point, it will be instantly teleported to another warp point with the same number. Warp points are connected in pairs - each pair should have the same number.

Example level with warp points:
```
##########
#O.....1.#
#........#
#........#
#1......G#
##########
```

In this example, when the ball touches either '1', it will be teleported to the other '1'.
