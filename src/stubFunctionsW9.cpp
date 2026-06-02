/* stubFunctionsW9.cpp */

#include <vector>
#include <string.h>
#include <stdio.h>

#include <stdlib.h> // used for rand() and srand() functions
#include <time.h> // used for srand() arg specification

#include "cards.hpp"
#include "data.hpp"

PILE oneShuffle(PILE deck){
    int nCards = deck.size(); // should be equal to numCards, but just in case...

    // Creating piles and new deck
    PILE pile1;
    pile1.reserve(nCards/2);
    PILE pile2;
    pile2.reserve((nCards+1)/2);
    PILE answer;
    answer.reserve(nCards);

    // Building piles
    for(int i=0; i<nCards; i++){
        if(i<nCards/2){
            pile1.emplace_back(deck[i].getValue(), deck[i].getSuit());
        } else{
            pile2.emplace_back(deck[i].getValue(), deck[i].getSuit());
        }
    }

    /* Merging piles into the new deck */
    int pile1Count = 0;
    int pile2Count = 0;
    double randNum = 0;
    // Initial merge
    while(pile1Count < nCards/2 && pile2Count < (nCards+1)/2){
        // Pick random number in range [0, 1] (technically could be RAND_MAX / RAND_MAX)
        randNum = (double)rand() / RAND_MAX;
        // If number less than 0.5 put Card from pile1[pile1Count] into answer deck
            // and increment pile1Count;
        if(randNum < 0.5){
            answer.emplace_back(pile1[pile1Count].getValue(), pile1[pile1Count].getSuit());
            pile1Count++;
        }
        // If number greater than or equal to 0.5 put card from pile2[pile2Count] into answer deck
            // and increment pile2Count
        else if(randNum >= 0.5){
            answer.emplace_back(pile2[pile2Count].getValue(), pile2[pile2Count].getSuit());
            pile2Count++;
        } else{
            printf("ERROR:  Failed to generate an adequate random number.");
            return answer;
        }
    }
    // Handling leftover cards
    if(pile1Count < nCards/2){ // pile 1 has leftover cards
        // add leftover cards to answer deck
        for(int i=pile1Count; i<nCards/2; i++){
            answer.emplace_back(pile1[i].getValue(), pile1[i].getSuit());
        }
    } else if(pile2Count < (nCards+1)/2){ // pile 2 has leftover cards
        // add leftover cards to leftover deck
        for(int i=pile2Count; i<(nCards+1)/2; i++){
            answer.emplace_back(pile2[i].getValue(), pile2[i].getSuit());
        }
    }

    // Wrapup
    return answer;
}


PILES deal(PILE deck, int numPlayers){
    // List of piles that will be returned
    PILES answer;


    // Error checking
    if(deck.size() <= 0 || numPlayers <= 0){
        printf("You do know that there has to be a deck of cards and at least one player to deal, right?\n");
        return answer;
    } else if(2*(unsigned int)numPlayers + 5 > deck.size()){// using unsigned int typecase to get a warning message from the compiler to go away
        printf("ERROR:  Not enough cards to support the number of players!\n");
        return answer;
    }

    // Creating entries in list of piles that will be returned
    answer.resize(numPlayers+2); // number of piles reflects the number of players, the community cards, and the leftover cards (in that order)
                                 // NOTE:  Apparently resize() has to be used instead of reserve() b/c reserve() doesn't actually create the piles
                                 // (This wasn't an issue w/ PILE though b/c these elems would then be created; however, in this code, each PILE is not directly created)


    /* Player piles */
    unsigned int cardNum = 0; // card number in the deck
    // ZZZ:  Not sure this is completely necessary?
    for(int h=0; h<numPlayers; h++){ // reserving each PILE
        answer[h].reserve(2);
    }

    for(int i=0; i<2; i++){ // each player gets two cards
        for(int j=0; j<numPlayers; j++){ // looping through players
            cardNum = i*numPlayers + j;
            answer[j].push_back(deck[cardNum]); // adds card to player hand
        }
    }

    /* Community pile */
    answer[numPlayers].reserve(5); // ZZZ:  Not sure this is completely necessary?
    for(int k=0; k<5; k++){ // five cards in the community pile
        cardNum++;
        answer[numPlayers].push_back(deck[cardNum]); // using numPlayers as index b/c indices [0, numPlayers) are used for the player piles
    }

    /* The rest of the cards */
    cardNum++;
    while(cardNum < deck.size()){
        answer[numPlayers+1].push_back(deck[cardNum]); // using numPlayers+1 as index b/c it is the last index for reserve(numPlayers+2)
        cardNum++;
    }

    // Wrapup
    return answer;
}