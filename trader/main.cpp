#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <string>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <map>
#include "json.hpp"

using json = nlohmann::json;

// Screen dimensions
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

// SDL Globals
SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
TTF_Font* gFont = NULL;
SDL_Color textColor = { 255, 255, 255, 255 }; // White

// --- Data Structures ---
struct GameSettings {
    std::string game_title;
    int initial_money, initial_health, max_health, initial_hold_space;
    int time_units_per_day, hotel_cost, hotel_hp_recovery, camp_hp_recovery;
    double camp_encounter_chance;
};

struct Quest {
    std::string name, description;
    int time_cost, min_hp_loss, max_hp_loss;
    double encounter_chance;
};

struct Town {
    std::string name, description;
    int id;
    std::vector<Quest> quests;
};

struct Item {
    std::string name, type, effect_description;
    int base_price;
    json data; // Flexible data field for hp_recovery, damage, etc.
};

struct Player {
    int health, max_health, money, hold_space;
    std::map<std::string, int> inventory;
};

// --- Global Game Variables ---
GameSettings gameSettings;
std::vector<Town> towns;
std::vector<Item> item_templates;
Player player;

enum class GameState {
    MainMenu,
    InTown,
    ViewStore,
    ViewQuests,
    OnQuest,
    Inventory,
    Traveling,
    Resting,
    GameOver,
    ErrorScreen
};

GameState currentState = GameState::MainMenu;
int current_day = 1;
int current_time = 0; // Time units elapsed in the current day
int current_town_index = 0;
std::string errorMessage = "";

// Menu selection
int mainMenu_selectedOption = 0;
int inTown_selectedOption = 0;
int resting_selectedOption = 0;
int quests_selectedOption = 0;
int inventory_selectedOption = 0;
int travel_selectedOption = 0;
std::vector<std::string> outcome_log;

// Store state
int store_selectedOption = 0;
std::vector<Item> current_store_items;
json quest_rewards_json;
std::map<std::string, int> current_store_prices;
std::map<std::string, int> current_store_quantities;

// --- Core Functions ---
bool initSDL();
bool loadGameData();
void initializeNewGame();
void generateStoreInventory();
void advanceTime(int units);
void closeSDL();
void renderText(const std::string& text, int x, int y, bool centered = false);

// --- Game State Rendering ---
void renderMainMenu();
void renderInTown();
void renderResting();
void renderStore();
void renderQuests();
void renderQuestOutcome();
void renderInventory();
void renderTraveling();
void renderUI();

// --- Game State Input Handling ---
void handleInput(SDL_Event& e, bool& quit);
void handleInput_MainMenu(SDL_Event& e, bool& quit);
void handleInput_InTown(SDL_Event& e);
void handleInput_Resting(SDL_Event& e);
void handleInput_Store(SDL_Event& e);
void handleInput_Quests(SDL_Event& e);
void handleInput_QuestOutcome(SDL_Event& e);
void handleInput_Inventory(SDL_Event& e);
void handleInput_Traveling(SDL_Event& e);

// --- Main Game Loop ---
int main(int argc, char* args[]) {
    if (!initSDL()) return 1;

    if (!loadGameData()) {
        currentState = GameState::ErrorScreen;
    } else {
        SDL_SetWindowTitle(gWindow, gameSettings.game_title.c_str());
    }

    srand(static_cast<unsigned int>(time(0)));
    bool quit = false;
    SDL_Event e;

    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) quit = true;
            handleInput(e, quit);
        }

        SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0xFF);
        SDL_RenderClear(gRenderer);

        switch (currentState) {
            case GameState::MainMenu: renderMainMenu(); break;
            case GameState::InTown: renderInTown(); break;
            case GameState::Resting: renderResting(); break;
            case GameState::ViewStore: renderStore(); break;
            case GameState::ViewQuests: renderQuests(); break;
            case GameState::OnQuest: renderQuestOutcome(); break;
            case GameState::Inventory: renderInventory(); break;
            case GameState::Traveling: renderTraveling(); break;
            case GameState::ErrorScreen:
                renderText("Error", 0, 50, true);
                renderText(errorMessage, 10, 150);
                renderText("Press Q to Quit", 0, 250, true);
                break;
            // TODO: Add other game state render calls
            default: renderText("State Not Implemented", 0, 200, true); break;
        }

        if (currentState != GameState::MainMenu && currentState != GameState::ErrorScreen) {
            renderUI(); // Render common UI elements like stats
        }

        SDL_RenderPresent(gRenderer);
        SDL_Delay(16);
    }

    closeSDL();
    return 0;
}

// --- Function Implementations ---

bool initSDL() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) return false;
    gWindow = SDL_CreateWindow("Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!gWindow) return false;
    gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
    if (!gRenderer) return false;
    if (TTF_Init() == -1) return false;
    gFont = TTF_OpenFont("VCR_OSD_MONO.ttf", 20);
    if (!gFont) return false;
    return true;
}

void closeSDL() {
    TTF_CloseFont(gFont);
    SDL_DestroyRenderer(gRenderer);
    SDL_DestroyWindow(gWindow);
    TTF_Quit();
    SDL_Quit();
}

bool loadGameData() {
    std::ifstream f("data.json");
    if (!f.is_open()) {
        errorMessage = "Could not open data.json";
        return false;
    }
    try {
        json data = json::parse(f);
        auto& s = data["game_settings"];
        gameSettings = {
            s["game_title"],
            s["initial_money"], s["initial_health"], s["max_health"], s["initial_hold_space"],
            s["time_units_per_day"], s["hotel_cost"], s["hotel_hp_recovery"], s["camp_hp_recovery"],
            s["camp_encounter_chance"]
        };

        for (const auto& town_json : data["towns"]) {
            Town t;
            t.name = town_json["name"];
            t.description = town_json.value("description", "");
            t.id = town_json["id"];
            for (const auto& quest_json : town_json["quests"]) {
                t.quests.push_back({quest_json["name"], quest_json["description"], quest_json["time_cost"], quest_json["min_hp_loss"], quest_json["max_hp_loss"], quest_json["encounter_chance"]});
            }
            towns.push_back(t);
        }

        for (const auto& item_json : data["items"]) {
            Item newItem;
            newItem.name = item_json["name"];
            newItem.type = item_json["type"];
            newItem.effect_description = item_json.value("effect_description", "No description.");
            newItem.base_price = item_json["base_price"];
            newItem.data = item_json.value("data", json::object());
            item_templates.push_back(newItem);
        }
        if (data.contains("quest_rewards")) {
            quest_rewards_json = data["quest_rewards"];
        }

    } catch (json::exception& e) {
        errorMessage = "JSON Error: " + std::string(e.what());
        return false;
    }
    return true;
}

void initializeNewGame() {
    player.health = gameSettings.initial_health;
    player.max_health = gameSettings.max_health;
    player.money = gameSettings.initial_money;
    player.hold_space = gameSettings.initial_hold_space;
    player.inventory.clear();
    current_day = 1;
    current_time = 0;
    current_town_index = 0;
}

void advanceTime(int units) {
    current_time += units;
    if (current_time >= gameSettings.time_units_per_day) {
        current_day += current_time / gameSettings.time_units_per_day;
        current_time %= gameSettings.time_units_per_day;
    }
}

void renderText(const std::string& text, int x, int y, bool centered) {
    if (!gFont) return;
    SDL_Surface* surface = TTF_RenderText_Solid(gFont, text.c_str(), textColor);
    if (!surface) return;
    SDL_Texture* texture = SDL_CreateTextureFromSurface(gRenderer, surface);
    if (!texture) {
        SDL_FreeSurface(surface);
        return;
    }
    SDL_Rect destRect = { x, y, surface->w, surface->h };
    if (centered) {
        destRect.x = (SCREEN_WIDTH - surface->w) / 2;
    }
    SDL_RenderCopy(gRenderer, texture, NULL, &destRect);
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

void renderMainMenu() {
    renderText(gameSettings.game_title, 0, 50, true);
    renderText((mainMenu_selectedOption == 0 ? "> " : "") + std::string("New Game"), 0, 200, true);
    renderText((mainMenu_selectedOption == 1 ? "> " : "") + std::string("Quit"), 0, 230, true);
}

void renderInTown() {
    if (towns.empty()) return;
    renderText(towns[current_town_index].name, 0, 50, true);
    renderText(towns[current_town_index].description, 0, 80, true);

    const std::vector<std::string> options = {"Store", "Quests", "Inventory", "Travel", "Rest"};
    for (size_t i = 0; i < options.size(); ++i) {
        renderText((inTown_selectedOption == static_cast<int>(i) ? "> " : "") + options[i], 50, 180 + (i * 30));
    }
}

void renderResting() {
    renderText("Where to rest?", 0, 50, true);

    std::string hotel_option = "Hotel (Cost: " + std::to_string(gameSettings.hotel_cost) + " | HP Gain: " + std::to_string(gameSettings.hotel_hp_recovery) + ")";
    std::string camp_option = "Camp (Free | HP Gain: " + std::to_string(gameSettings.camp_hp_recovery) + " | " + std::to_string(static_cast<int>(gameSettings.camp_encounter_chance)) + "% Encounter)";
    std::string back_option = "Back to Town";

    renderText((resting_selectedOption == 0 ? "> " : "") + hotel_option, 50, 150);
    renderText((resting_selectedOption == 1 ? "> " : "") + camp_option, 50, 200);
    renderText((resting_selectedOption == 2 ? "> " : "") + back_option, 50, 250);
}

void renderUI() {
    // Top bar with player stats
    std::string health_str = "HP: " + std::to_string(player.health) + "/" + std::to_string(player.max_health);
    std::string money_str = "$" + std::to_string(player.money);
    std::string time_str = "Day " + std::to_string(current_day) + " - Time: " + std::to_string(current_time) + "/" + std::to_string(gameSettings.time_units_per_day);
    
    renderText(health_str, 10, 10);
    renderText(money_str, 200, 10);
    renderText(time_str, SCREEN_WIDTH - 250, 10);

    // Draw a line under the top bar
    SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 255);
    SDL_RenderDrawLine(gRenderer, 0, 40, SCREEN_WIDTH, 40);
}

void handleInput(SDL_Event& e, bool& quit) {
    if (e.type != SDL_KEYDOWN) return;

    switch (currentState) {
        case GameState::MainMenu: handleInput_MainMenu(e, quit); break;
        case GameState::InTown: handleInput_InTown(e); break;
        case GameState::Resting: handleInput_Resting(e); break;
        case GameState::ViewStore: handleInput_Store(e); break;
        case GameState::ViewQuests: handleInput_Quests(e); break;
        case GameState::OnQuest: handleInput_QuestOutcome(e); break;
        case GameState::Inventory: handleInput_Inventory(e); break;
        case GameState::Traveling: handleInput_Traveling(e); break;
        case GameState::ErrorScreen:
            if (e.key.keysym.sym == SDLK_q) quit = true;
            break;
        default: break;
    }
}

void handleInput_MainMenu(SDL_Event& e, bool& quit) {
    if (e.type != SDL_KEYDOWN) return;

    switch (e.key.keysym.sym) {
        case SDLK_UP:
            mainMenu_selectedOption = (mainMenu_selectedOption - 1 + 2) % 2;
            break;
        case SDLK_DOWN:
            mainMenu_selectedOption = (mainMenu_selectedOption + 1) % 2;
            break;
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
            if (mainMenu_selectedOption == 0) { // New Game
                initializeNewGame();
                inTown_selectedOption = 0; // Reset town menu selection
                currentState = GameState::InTown;
            } else if (mainMenu_selectedOption == 1) { // Quit
                quit = true;
            }
            break;
        case SDLK_q:
             quit = true;
             break;
    }
}

void handleInput_InTown(SDL_Event& e) {
    if (e.type != SDL_KEYDOWN) return;
    const int numOptions = 5;

    switch (e.key.keysym.sym) {
        case SDLK_UP:
            inTown_selectedOption = (inTown_selectedOption - 1 + numOptions) % numOptions;
            break;
        case SDLK_DOWN:
            inTown_selectedOption = (inTown_selectedOption + 1) % numOptions;
            break;
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
            switch (inTown_selectedOption) {
                case 0: // Store
                    generateStoreInventory();
                    store_selectedOption = 0;
                    currentState = GameState::ViewStore;
                    break;
                case 1: // Quests
                    quests_selectedOption = 0;
                    currentState = GameState::ViewQuests;
                    break;
                case 2: // Inventory
                    inventory_selectedOption = 0;
                    currentState = GameState::Inventory;
                    break;
                case 3: // Travel
                    travel_selectedOption = 0;
                    currentState = GameState::Traveling;
                    break;
                case 4: // Rest
                    resting_selectedOption = 0;
                    currentState = GameState::Resting;
                    break;
            }
            break;
        case SDLK_ESCAPE:
            // In the future, this might go to a game menu instead of quitting
            break;
    }
}

void handleInput_Resting(SDL_Event& e) {
    if (e.type != SDL_KEYDOWN) return;
    const int numOptions = 3;

    switch (e.key.keysym.sym) {
        case SDLK_UP:
            resting_selectedOption = (resting_selectedOption - 1 + numOptions) % numOptions;
            break;
        case SDLK_DOWN:
            resting_selectedOption = (resting_selectedOption + 1) % numOptions;
            break;
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
            switch (resting_selectedOption) {
                case 0: // Hotel
                    if (player.money >= gameSettings.hotel_cost) {
                        player.money -= gameSettings.hotel_cost;
                        player.health = player.max_health;
                        int time_to_advance = (gameSettings.time_units_per_day - current_time);
                        advanceTime(time_to_advance);
                        currentState = GameState::InTown;
                    }
                    break;
                case 1: // Camp
                    {
                        int time_to_advance = (gameSettings.time_units_per_day - current_time);
                        advanceTime(time_to_advance);
                        player.health = std::min(player.max_health, player.health + gameSettings.camp_hp_recovery);
                        if ((static_cast<float>(rand()) / RAND_MAX) < gameSettings.camp_encounter_chance) {
                            int hp_loss = 5 + (rand() % 6); // 5-10 HP loss
                            player.health -= hp_loss;
                            outcome_log.clear();
                            outcome_log.push_back("Attacked by zombies while sleeping!");
                            outcome_log.push_back("Lost " + std::to_string(hp_loss) + " HP.");
                            if (player.health <= 0) {
                                player.health = 0;
                                currentState = GameState::GameOver;
                            } else {
                                currentState = GameState::OnQuest; // Reuse outcome screen
                            }
                        } else {
                            currentState = GameState::InTown;
                        }
                    }
                    break;
                case 2: // Back to Town
                    currentState = GameState::InTown;
                    break;
            }
            break;
        case SDLK_ESCAPE:
            currentState = GameState::InTown;
            break;
    }
}

void generateStoreInventory() {
    current_store_items.clear();
    current_store_prices.clear();
    current_store_quantities.clear();

    for (const auto& item_template : item_templates) {
        if ((rand() % 100) < 70) { // 70% chance for an item to be in the store
            current_store_items.push_back(item_template);
            float price_modifier = 1.0f + ((rand() % 61) - 30) / 100.0f; // -30% to +30%
            current_store_prices[item_template.name] = static_cast<int>(item_template.base_price * price_modifier);
            current_store_quantities[item_template.name] = (rand() % 10) + 1;
        }
    }
}

void renderStore() {
    renderText("Store", 0, 50, true);
    renderText("Item", 50, 120);
    renderText("Price", 300, 120);
    renderText("Qty", 400, 120);
    renderText("Your Qty", 500, 120);

    for (size_t i = 0; i < current_store_items.size(); ++i) {
        const auto& item = current_store_items[i];
        std::string prefix = (store_selectedOption == static_cast<int>(i) ? "> " : "  ");
        renderText(prefix + item.name, 20, 150 + (i * 25));
        renderText("$" + std::to_string(current_store_prices[item.name]), 300, 150 + (i * 25));
        renderText(std::to_string(current_store_quantities[item.name]), 400, 150 + (i * 25));
        renderText(std::to_string(player.inventory.count(item.name) ? player.inventory.at(item.name) : 0), 500, 150 + (i * 25));
    }

    if (!current_store_items.empty() && store_selectedOption < current_store_items.size()) {
        const auto& selected_item = current_store_items[store_selectedOption];
        renderText("Effect: " + selected_item.effect_description, 50, SCREEN_HEIGHT - 70);
    }

    renderText("Enter: Buy, Esc: Back", 50, SCREEN_HEIGHT - 40);
}

void handleInput_Store(SDL_Event& e) {
    if (e.type != SDL_KEYDOWN) return;

    if (current_store_items.empty()) {
        if (e.key.keysym.sym == SDLK_ESCAPE) {
            currentState = GameState::InTown;
        }
        return;
    }

    const int numOptions = current_store_items.size();
    switch (e.key.keysym.sym) {
        case SDLK_UP:
            store_selectedOption = (store_selectedOption - 1 + numOptions) % numOptions;
            break;
        case SDLK_DOWN:
            store_selectedOption = (store_selectedOption + 1) % numOptions;
            break;
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
            {
                const auto& selected_item = current_store_items[store_selectedOption];
                int price = current_store_prices[selected_item.name];
                if (player.money >= price && current_store_quantities[selected_item.name] > 0) {
                    player.money -= price;
                    player.inventory[selected_item.name]++;
                    current_store_quantities[selected_item.name]--;
                }
            }
            break;
        case SDLK_ESCAPE:
            currentState = GameState::InTown;
            break;
    }
}

void renderQuests() {
    renderText("Quests", 0, 50, true);
    const auto& current_quests = towns[current_town_index].quests;
    if (current_quests.empty()) {
        renderText("No quests available right now.", 0, 150, true);
    } else {
        for (size_t i = 0; i < current_quests.size(); ++i) {
            renderText((quests_selectedOption == static_cast<int>(i) ? "> " : "") + current_quests[i].name, 50, 120 + (i * 25));
        }
        const auto& selected_quest = current_quests[quests_selectedOption];
        renderText(selected_quest.description, 50, 300);
        renderText("Time: " + std::to_string(selected_quest.time_cost) + " units", 50, 330);
        renderText("Risk: " + std::to_string(selected_quest.min_hp_loss) + "-" + std::to_string(selected_quest.max_hp_loss) + " HP loss", 50, 360);
    }
    renderText("ENTER to Accept, ESC to leave", 50, SCREEN_HEIGHT - 40);
}

void handleInput_Quests(SDL_Event& e) {
    if (e.type != SDL_KEYDOWN) return;
    const auto& current_quests = towns[current_town_index].quests;
    if (current_quests.empty()) {
        if (e.key.keysym.sym == SDLK_ESCAPE) {
            currentState = GameState::InTown;
        }
        return;
    }

    const int numOptions = current_quests.size();
    switch (e.key.keysym.sym) {
        case SDLK_UP:
            quests_selectedOption = (quests_selectedOption - 1 + numOptions) % numOptions;
            break;
        case SDLK_DOWN:
            quests_selectedOption = (quests_selectedOption + 1) % numOptions;
            break;
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
            {
                const auto& quest = current_quests[quests_selectedOption];
                advanceTime(quest.time_cost);

                outcome_log.clear();
                outcome_log.push_back("Spent " + std::to_string(quest.time_cost) + " hours on the quest.");

                if ((rand() % 100) < (quest.encounter_chance * 100)) {
                    int hp_loss = quest.min_hp_loss + (rand() % (quest.max_hp_loss - quest.min_hp_loss + 1));
                    player.health -= hp_loss;
                    outcome_log.push_back("Lost " + std::to_string(hp_loss) + " HP in an encounter.");
                    if (player.health <= 0) {
                        player.health = 0;
                        currentState = GameState::GameOver;
                        return;
                    }
                } else {
                    outcome_log.push_back("No incidents reported.");
                }

                outcome_log.push_back(""); // Spacer
                outcome_log.push_back("Rewards:");
                bool has_rewards = false;
                if (quest_rewards_json.contains(quest.name)) {
                    for (const auto& reward_info : quest_rewards_json[quest.name]) {
                        if ((static_cast<float>(rand()) / RAND_MAX) < reward_info["probability"].get<float>()) {
                            int quantity = reward_info["min_quantity"].get<int>() + (rand() % (reward_info["max_quantity"].get<int>() - reward_info["min_quantity"].get<int>() + 1));
                            if (quantity > 0) {
                                std::string item_name = reward_info["item_name"].get<std::string>();
                                player.inventory[item_name] += quantity;
                                outcome_log.push_back("- " + std::to_string(quantity) + "x " + item_name);

                                std::string desc = "";
                                for (const auto& tpl : item_templates) {
                                    if (tpl.name == item_name) {
                                        desc = tpl.effect_description;
                                        break;
                                    }
                                }
                                if (!desc.empty()) {
                                    outcome_log.push_back("  (" + desc + ")");
                                }
                                has_rewards = true;
                            }
                        }
                    }
                }
                if (!has_rewards) {
                    outcome_log.push_back("- Nothing of value found.");
                }

                currentState = GameState::OnQuest;
            }
            break;
        case SDLK_ESCAPE:
            currentState = GameState::InTown;
            break;
    }
}

void renderQuestOutcome() {
    renderText("Event Report", 0, 50, true);
    int y_pos = 120;
    for (const auto& line : outcome_log) {
        renderText(line, 50, y_pos);
        y_pos += 25;
    }
    renderText("Press any key to continue...", 50, SCREEN_HEIGHT - 40);
}

void handleInput_QuestOutcome(SDL_Event& e) {
    if (e.type == SDL_KEYDOWN) {
        currentState = GameState::InTown;
    }
}

void renderInventory() {
    renderText("Inventory", 0, 50, true);
    if (player.inventory.empty()) {
        renderText("Your inventory is empty.", 0, 150, true);
    } else {
        int y_pos = 120;
        int current_item_index = 0;
        std::vector<std::string> player_items;
        for(const auto& pair : player.inventory) {
            player_items.push_back(pair.first);
        }

        for (const auto& item_name : player_items) {
            std::string prefix = (inventory_selectedOption == current_item_index ? "> " : "  ");
            std::string line = prefix + item_name + " (x" + std::to_string(player.inventory.at(item_name)) + ")";
            renderText(line, 50, y_pos);
            y_pos += 25;
            current_item_index++;
        }

        if (!player_items.empty()) {
            std::string selected_item_name = player_items[inventory_selectedOption];
            const Item* selected_item_template = nullptr;
            for (const auto& tpl : item_templates) {
                if (tpl.name == selected_item_name) {
                    selected_item_template = &tpl;
                    break;
                }
            }
            if (selected_item_template) {
                renderText("Effect: " + selected_item_template->effect_description, 50, SCREEN_HEIGHT - 70);
            }
        }
    }
    renderText("Enter: Use, Esc: Back", 50, SCREEN_HEIGHT - 40);
}

void handleInput_Inventory(SDL_Event& e) {
    if (e.type != SDL_KEYDOWN) return;
    if (player.inventory.empty()) {
        if (e.key.keysym.sym == SDLK_ESCAPE) {
            currentState = GameState::InTown;
        }
        return;
    }

    const int numOptions = player.inventory.size();
    switch (e.key.keysym.sym) {
        case SDLK_UP:
            inventory_selectedOption = (inventory_selectedOption - 1 + numOptions) % numOptions;
            break;
        case SDLK_DOWN:
            inventory_selectedOption = (inventory_selectedOption + 1) % numOptions;
            break;
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
            // Use item logic here
            {
                std::vector<std::string> player_items;
                for(const auto& pair : player.inventory) {
                    player_items.push_back(pair.first);
                }
                std::string item_name = player_items[inventory_selectedOption];

                const Item* item_template = nullptr;
                for(const auto& tpl : item_templates) {
                    if (tpl.name == item_name) {
                        item_template = &tpl;
                        break;
                    }
                }

                if (item_template && (item_template->type == "Food" || item_template->type == "Medicine" || item_template->type == "food")) {
                    player.health += item_template->data.value("hp_recovery", 0);
                    if (player.health > player.max_health) {
                        player.health = player.max_health;
                    }
                    player.inventory[item_name]--;
                    if (player.inventory[item_name] <= 0) {
                        player.inventory.erase(item_name);
                        if (inventory_selectedOption >= player.inventory.size() && !player.inventory.empty()) {
                            inventory_selectedOption = player.inventory.size() - 1;
                        }
                    }
                }
            }
            break;
        case SDLK_ESCAPE:
            currentState = GameState::InTown;
            break;
    }
}

void renderTraveling() {
    renderText("Travel to...", 0, 50, true);
    for (size_t i = 0; i < towns.size(); ++i) {
        if (static_cast<int>(i) == current_town_index) continue;
        std::string prefix = (travel_selectedOption == static_cast<int>(i) ? "> " : "  ");
        renderText(prefix + towns[i].name, 50, 150 + (i * 30));
    }
    renderText("Esc: Cancel", 50, SCREEN_HEIGHT - 40);
}

void handleInput_Traveling(SDL_Event& e) {
    if (e.type != SDL_KEYDOWN) return;
    int travel_target = -1;
    std::vector<int> town_indices;
    for(size_t i = 0; i < towns.size(); ++i) {
        if(static_cast<int>(i) != current_town_index) {
            town_indices.push_back(i);
        }
    }

    const int numOptions = town_indices.size();
    if (numOptions == 0) {
        if (e.key.keysym.sym == SDLK_ESCAPE) currentState = GameState::InTown;
        return;
    }

    switch (e.key.keysym.sym) {
        case SDLK_UP:
            travel_selectedOption = (travel_selectedOption - 1 + numOptions) % numOptions;
            break;
        case SDLK_DOWN:
            travel_selectedOption = (travel_selectedOption + 1) % numOptions;
            break;
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
            travel_target = town_indices[travel_selectedOption];
            if (travel_target != -1) {
                current_town_index = travel_target;
                advanceTime(4); // Travel takes 4 time units
                currentState = GameState::InTown;
            }
            break;
        case SDLK_ESCAPE:
            currentState = GameState::InTown;
            break;
    }
}





