#include "botDecisions.hpp"
#include "data.hpp"
#include "cards.hpp"
#include <iostream>
#include <cstdlib>

int main() {
    srand(1);

    GAMESTATE gameState;

    gameState.numPlayers = 2;
    gameState.round = Preflop;
    gameState.callAmount = 10;

    gameState.players.resize(gameState.numPlayers);
    gameState.allCards.resize(gameState.numPlayers + 2);

    gameState.players[0].playerNum = 0;
    gameState.players[0].playerType = Computer;
    gameState.players[0].score = 100;
    gameState.players[0].bet = 0;
    gameState.players[0].isInHand = 1;

    gameState.players[1].playerNum = 0;
    gameState.players[1].playerType = Computer;
    gameState.players[1].score = 100;
    gameState.players[1].bet = 0;
    gameState.players[1].isInHand = 1;

    gameState.allCards[0].push_back(Card(Ace, Spades));
    gameState.allCards[0].push_back(Card(Ace, Hearts));

    int deckIndex = gameState.allCards.size() - 1;

    for (int s = Hearts; s <= Spades; s++) {
        for (int v = Two; v <= Ace; v++) {
            if (v == Ace && (s == Hearts || s == Spades)) {
                continue;
            }
            gameState.allCards[deckIndex].push_back(Card(v,s));
        }
    }

    double odds = calculateOdds(gameState, 0);
    int action = decideAction(gameState, 0);
    int raiseAmount = getRaiseAmount(gameState, 0);

    std::cout << "Odds: " << odds << std::endl;
    std::cout << "Action: " << action << std::endl;
    std::cout << "Raise amount: " << raiseAmount << std::endl;

    return 0;
}