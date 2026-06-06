#include "login.hpp"
#include <iostream>

std::vector<PlayerData>* get_test_players() {
    auto* players = new std::vector<PlayerData>();
    
    std::cout << "\n--- End Screen Name Tester ---\n";
    
    for (int i = 1; i <= 3; ++i) {
        PlayerData p;
        std::cout << "Enter name for Player " << i << ": ";
        std::getline(std::cin, p.name);
        
        if (p.name.empty()) p.name = "Player_" + std::to_string(i);
        
        // Mock data for chips
        p.chips = 1000000 / i; 
        p.bonus = (i == 1) ? "(+ $4,250)" : "";
        
        players->push_back(p);
    }
    return players;
}
