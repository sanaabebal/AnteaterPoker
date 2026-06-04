/* botDecisions.cpp */

/* !!! MEANS I NEED YOU GUYS TO DO/FINISH !!! */

/* !!! needs handHierarchy in scoreCalc.cpp to be completed !!! */
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
            if (temp.players[j].isInHand == 0) continue;  //old was [sim]

            int oppScore = handHierarchy(temp.allCards, j);

            if (oppScore > playerScore) {
                lost = true;
                break;
            }
            else if (oppScore == playerScore) {
                tied = true;
            }
        }
        // update sim results
        if (!lost && !tied) wins++;
        else if (!lost && tied) ties++;
    }
    // return probability
    return (wins + ties * 0.5) / (double)simulations;
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
    int Bluffing = bluffing(gameState, playerNum, wr);  //calling bluffing function
    int points = gameState.players[playerNum].score;

    //all in were if called amount is greater than the points bot has, it can fold or go all in
    if(gameState.callAmount >= points){
	    if(wr > 0.35 || Bluffing){
		    return AllIn;
	    }
	    return Fold;
    }
    //where bluffing happens, if bot can raise then raise, if not see if checking is free as its not worth calling when bluffing
    if(Bluffing){
	    if(canRaise)
		    return Raise;
	    if(validCheck(gameState, playerNum))
		    return Check;
	    else
		    return Fold;
    }

    // criteria if check available 
    if (validCheck(gameState, playerNum)) {
        if (gameState.stage == Preflop) {
            if (wr > 0.7 && canRaise) return Raise;
            else {
                if (rp < 5 && canRaise) return Raise;
                else return Check;
            }
        }
        if (gameState.stage == Flop) {
            if (wr > 0.75 && canRaise) return Raise;
            else {
                if (rp < 15 && canRaise) return Raise;
                else return Check;
            }
        }
        if (gameState.stage == Turn) {
            if (wr > 0.75 && canRaise) return Raise;
            else {
                if (rp < 10 && canRaise) return Raise;
                else return Check;
            }
        }
        if (gameState.stage == River) {
            if (wr > 0.75 && canRaise) return Raise;
            else {
                if (rp < 5 && canRaise) return Raise;
                else return Check;
            }
        }
    }
    // criteria if check is not available
    if (gameState.stage == Preflop) {
        if (wr > 0.75 && canRaise) return Raise;
        else if (wr > 0.4 && canCall) return Call;
        else {
            if (rp < 5 && canRaise) return Raise;
            else if (rp < 20 && canCall) return Call;
            else return Fold;
        }
    }
    if (gameState.stage == Flop) {
        if (wr > 0.7 && canRaise) return Raise;
        else if (wr > 0.35 && canCall) return Call;
        else {
            if (rp < 10 && canRaise) return Raise;
            else if (rp < 30 && canCall) return Call;
            else return Fold;
        }
    }
    if (gameState.stage == Turn) {
        if (wr > 0.65 && canRaise) return Raise;
        else if (wr > 0.3 && canCall) return Call;
        else {
            if (rp < 8 && canRaise) return Raise;
            else if (rp < 20 && canCall) return Call;
            else return Fold;
        }
    }
    if (gameState.stage == River) {
        if (wr > 0.6 && canRaise) return Raise;
        else if (wr > 0.25 && canCall) return Call;
        else {
            if (rp < 5 && canRaise) return Raise;
            else if (wr < 10 && canCall) return Call;
            else Fold;
        }
    }
    // if no above options chosen, player must be big/small blind
    return Check;  //changed from 0 to check, change back if it doesnt work!!
}


/* Determines the amount of points to raise by */
int getRaiseAmount(GAMESTATE gameState, int playerNum, double wr, int isBluff) {
    int points = gameState.players[playerNum].score;
    int raiseAmount;

    if(isBluff) raiseAmount = points/4;
    else if (wr >= 0.95) raiseAmount = points; // all in
    else if (wr >= 0.85) raiseAmount = points/2;
    else if (wr >= 0.75) raiseAmount = points/4;
    else raiseAmount = gameState.callAmount;

    // checks to make sure bot does not try to raise more than what it has
    if (raiseAmount > points) raiseAmount = points;

    return raiseAmount;
}

/*start of bluffing function*/
int bluffing(GAMESTATE gameState, int playerNum, double wr){

	int rp = rand() %100;
	int activeplayers = 0;
	int aggressive = 0;
	
	//checks how many players are active
	for(int i = 0; i < gameState.numPlayers; i++){
		if(gameState.players[i].isInHand)
			activeplayers++;
	}
	//will not bluff with many players on the table
	if(activeplayers > 3)
		return 0;
	//for when it's a 1v1
	aggressive = (activeplayers == 2) ? 5 : 0;

	//bluff for when the bot has a garbage hand
	if(wr < 0.12){
		if(gameState.stage == Flop && rp < 10 + aggressive)
			return 1;
		if(gameState.stage == Turn && rp < 5 + aggressive)
			return 1;
		if(gameState.stage == River && rp < 3 + aggressive)
			return 1;
	}
	//bluff for when hand is better
	else if(wr >= 0.12 && wr < 0.35){
		if(gameState.stage == Flop && rp < 25 + aggressive)
			return 1;
		if(gameState.stage == Turn && rp < 15 + aggressive)
			return 1;
	}
	return 0;
}

