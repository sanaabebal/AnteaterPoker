#ifndef LOGIN_HPP
#define LOGIN_HPP

#include <string>
#include <vector>

struct PlayerData {
    std::string name;
    long chips;
    std::string bonus;
};

// Function to prompt terminal and return a list of players
std::vector<PlayerData>* get_test_players();

#endif
