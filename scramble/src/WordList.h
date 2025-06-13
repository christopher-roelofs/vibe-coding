#ifndef WORDLIST_H
#define WORDLIST_H

#include <string>
#include <vector>
#include "json.hpp" // nlohmann/json

using json = nlohmann::json;

struct WordEntry {
    std::string word;
    std::string hint;
};

struct WordList {
    std::string name;
    std::string author;
    std::string date;
    std::string description;
    std::vector<WordEntry> words;

    static bool loadFromFile(const std::string& filepath, WordList& out_word_list);
};

#endif // WORDLIST_H
