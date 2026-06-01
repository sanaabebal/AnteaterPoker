/* botDecisions.cpp */

/* !!! MEANS I NEED YOU GUYS TO DO/FINISH !!! */

/* !!! needs handHierarchy in scoreCalc.cpp to be completed !!! */
/* !!! also need a All-in option*/
/* I also want to add a bluffing state  
   Basically once the bot bluffs, it commits and keeps up the bluff 
   unless things other players scare it out (ex: keep raising) 
   !!! would be helpful if bluff boolean is added to player struct !!! */

#include "botDecisions.hpp"
#include "cards.hpp"
#include "data.hpp"
#include "scoreCalc.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <random>

// this function calculates the odds of player by simulating all possible scenarios
// returns a percentage of how often player would win out of 10000 randomly
// simulated games (ties count as half a win)
double calculateOdds(GAMESTATE gameState, int playerNum) {
    int wins = 0;
    int ties = 0;
    int simulations = 10000; // subject to change

    for (int sim; sim < simulations; sim++) {
        GAMESTATE temp = gameState;

        int communityIndex = temp.allCards.size() - 2;
        int deckIndex = temp.allCards.size() - 1;

        // create fresh deck 
        PILE deck = temp.allCards[deckIndex];

        // deal opponent hands
        for (int i = 0; i < temp.numPlayers; i++) {
            if (i == playerNum) continue;
            if (temp.players[i].isInHand == 0) continue;

            Card c1 = drawRandomCard(deck);
            Card c2 = drawRandomCard(deck);

            temp.allCards[i].clear();
            temp.allCards[i].push_back(c1);
            temp.allCards[i].push_back(c2);
        }

        // community cards, find out how many already exist
        while (temp.allCards[communityIndex].sizeOf() < 5) {
            Card c = drawRandomCard(deck);
            temp.allCards[communityIndex].push_back(c);
        }

        // update leftover deck in temp
        temp.allCards[deckIndex] = deck;

        // evaluate player
        int playerScore = handHierarchy(temp.allCards, playerNum);

        bool lost = false;
        bool tied = false;

        // compare to all opponents
        for (int j = 0; j < temp.numPlayers; j++) {
            if (j == playerNum) continue;
            if (temp.players[sim].isInHand == 0) continue;

            int oppScore = handHierarchy(temp.allCards, j);

            if (oppScore > playerScore) {
                lost = true;
                break;
            }
            else if (oppScore == playerScore) {
                tied = true;
            }
        }
        // update results
        if (!lost && !tied) wins++;
        else if (!lost && tied) ties++;
    }
    // return probability
    return (wins + ties * 0.5) / simulations;
}

// draws a random card in deck
Card drawRandomCard(PILE &deck) {
    int index = rand() % deck.sizeOf();
    Card drawn = deck[index];
    deck.erase(deck.begin() + index);
    return drawn;
}

/* This function makes decision for bot player based on winrate from calculateOdds */
/* Side note: should only be called if is not eliminated and isn't small/big blind */
int decideAction(GAMESTATE gameState, int playerNum) {
    double wr = calculateOdds(gameState, playerNum);
    int rp = rand() % 100;
    int canRaise = validRaise(gameState, playerNum);
    int canCall = validCall(gameState, playerNum);

    // criteria if check available 
    if (validCheck(gameState, playerNum)) {
        if (gameState == Preflop) {
            if (wr > 0.7 && canRaise) return Raise;
            else {
                if (rp < 5 && canRaise) return Raise;
                else return Check;
            }
        }
        if (gameState == Flop) {
            if (wr > 0.75 && canRaise) return Raise;
            else {
                if (rp < 15 && canRaise) return Raise;
                else return Check;
            }
        }
        if (gameState == Turn) {
            if (wr > 0.75 && canRaise) return Raise;
            else {
                if (rp < 10 && canRaise) return Raise;
                else return Check;
            }
        }
        if (gameState == River) {
            if (wr > 75 && canRaise) return Raise;
            else {
                if (rp < 5 && canRaise) return Raise;
                else return Check;
            }
        }
    }
    // criteria if check is not available
    if (gameState == Preflop) {
        if (wr > 0.75 && canRaise) return Raise;
        else if (wr > 0.4 && canCall) return Call;
        else {
            if (rp < 5 && canRaise) return Raise;
            else if (rp < 20 && canCall) return Call;
            else return Fold;
        }
    }
    if (gameState == Flop) {
        if (wr > 0.7 && canRaise) return Raise;
        else if (wr > 0.35 && canCall) return Call;
        else {
            if (rp < 10 && canRaise) return Raise;
            else if (rp < 30 && canCall) return Call;
            else return Fold;
        }
    }
    if (gameState == Turn) {
        if (wr > 0.65 && canRaise) return Raise;
        else if (wr > 0.3 && canCall) return Call;
        else {
            if (rp < 8 && canRaise) return Raise;
            else if (rp < 20 && canCall) return Call;
            else return Fold;
        }
    }
    if (gameState == River) {
        if (wr > 0.6 && canRaise) return Raise;
        else if (wr > 0.25 && canCall) return Call;
        else {
            if (rp < 5 && canRaise) return Raise;
            else if (wr < 10 && canCall) return Call;
            else Fold;
        }
    }
    // if no above options chosen, player must be big/small blind
    return 0;
}


/* Determines the amount of points to raise by */
int getRaiseAmount(GAMESTATE gameState, int playerNum, double wr) {
    int points = gameState.players[playerNum].score;
    int raiseAmount;

    if (wr >= 0.95) raiseAmount = points; // all in
    else if (wr >= 0.85) raiseAmount = points/2;
    else if (wr >= 0.75) raiseAmount = points/4;
    else raiseAmount = gameState.callAmount;

    // if not enough points for above criteria, go all in
    if (raiseAmount > points) raiseAmount = points;

    return raiseAmount;
}