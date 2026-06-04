/* botDecisions.hpp */

#ifndef BOTDECISIONS_H
#define BOTDECISIONS_H

#include "cards.hpp"
#include "data.hpp"

// returns win rate % (out of # simulations)
double calculateOdds(GAMESTATE gameState, int playerNum);

// returns the bot's action (fold, check, raise, etc.)
int decideAction(GAMESTATE gameState, int playerNum);

// returns amount that will be raised
int getRaiseAmount(GAMESTATE gameState, int playerNum, double wr);

//handles bot bluffing
int bluffing(GAMESTATE gameState, int playerNum, double wr);

#endif
