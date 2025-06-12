# Wasteland Trader (A Zombie Survival RPG)

A simple zombie survival RPG implemented in C++ using SDL2.

## Gameplay

You are a trader in a post-apocalyptic wasteland. Your goal is to survive, complete quests, and perhaps even thrive.

- **Survival:** Manage your Health (HP), money, and inventory.
- **Towns:** Travel between different settlements, each with its own store and set of quests.
- **Quests:** Take on tasks like scavenging or clearing out zombies. Quests take time and carry risks, but offer valuable rewards.
- **Time Management:** Each day is divided into a set number of time units. Quests and other actions consume time.
- **Resting:** You can end the day by resting.
  - **Camp:** Free, but recovers less health and you might be attacked during the night.
  - **Hotel:** Costs money, but is safe and recovers more health.
- **Economy:** Buy and sell items like food, medicine, weapons, and materials at town stores.

## Dependencies

-   SDL2
-   SDL2_ttf
-   A C++ compiler (e.g., g++)
-   Make
-   `json.hpp` (from [nlohmann/json](https://github.com/nlohmann/json)). Download `json.hpp` and place it in the project root directory.

## Game Data File

The game uses a `data.json` file to store all game parameters, including towns, items, quests, and player settings. This file must be present for the game to run.

## Font

This game requires the `VCR_OSD_MONO.ttf` font file to be present in the same directory as the executable.

## Build & Run

1.  Ensure all dependencies are installed.
2.  Place `json.hpp` and `VCR_OSD_MONO.ttf` in the project root.
3.  Run `make` to build the executable (`dopewars`).
4.  Run `./dopewars` to play.

## Controls (Initial Plan)

- **In Town:**
  - **S**: Open Store
  - **Q**: View Quests
  - **T**: Travel to another town
  - **R**: Rest (end the day)
- **Menus:**
  - **Up/Down Arrows**: Navigate lists
  - **Enter**: Select
  - **Escape/Backspace**: Go back
- **Global:**
  - **Q**: Quit Game (from main menu)
