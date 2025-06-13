#include "WordList.h"
#include <fstream>
#include <iostream> // For error reporting

bool WordList::loadFromFile(const std::string& filepath, WordList& out_word_list) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open word list file: " << filepath << std::endl;
        return false;
    }

    try {
        json j = json::parse(file);

        out_word_list.name = j.value("name", "Unnamed List");
        out_word_list.author = j.value("author", "Unknown Author");
        out_word_list.date = j.value("date", "N/A");
        out_word_list.description = j.value("description", "No description.");

        if (j.contains("words") && j["words"].is_array()) {
            for (const auto& entry_json : j["words"]) {
                if (entry_json.is_object() && entry_json.contains("word") && entry_json["word"].is_string() && entry_json.contains("hint") && entry_json["hint"].is_string()) {
                    WordEntry entry;
                    entry.word = entry_json["word"].get<std::string>();
                    entry.hint = entry_json["hint"].get<std::string>();
                    if (!entry.word.empty()) { // Ensure word is not empty
                       out_word_list.words.push_back(entry);
                    }
                } else {
                    std::cerr << "Warning: Skipping invalid word entry in " << filepath << std::endl;
                }
            }
        }
        if (out_word_list.words.empty()){
            std::cerr << "Warning: Word list '" << out_word_list.name << "' from " << filepath << " contains no valid word entries." << std::endl;
            // Optionally, you could return false here if an empty word list is an error
        }

    } catch (json::parse_error& e) {
        std::cerr << "Error parsing JSON from file: " << filepath << "\n" << "Message: " << e.what() << "\n"
                  << "Exception id: " << e.id << "\n" << "Byte position of error: " << e.byte << std::endl;
        file.close();
        return false;
    } catch (json::type_error& e) {
        std::cerr << "Error in JSON structure (type error) in file: " << filepath << "\n" << "Message: " << e.what() << std::endl;
        file.close();
        return false;
    } catch (const std::exception& e) {
        std::cerr << "An unexpected error occurred while processing JSON file: " << filepath << "\n" << "Message: " << e.what() << std::endl;
        file.close();
        return false;
    }

    file.close();
    return true;
}
