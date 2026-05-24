/* testCards.cpp */

#include <stdio.h>
#include <vector>
#include <stdio.h>
#include <string.h>

#include "cards.hpp"

/* TESTING FUNCTIONS */
// helper function called by setupTestGameDeal()
int parseTestInput(char *oneInput){
    // Testing values
    if(strcmp(oneInput, "Two") == 0){ return Two; }
    if(strcmp(oneInput, "Three") == 0){ return Three; }
    if(strcmp(oneInput, "Four") == 0){ return Four; }
    if(strcmp(oneInput, "Five") == 0){ return Five; }
    if(strcmp(oneInput, "Six") == 0){ return Six; }
    if(strcmp(oneInput, "Seven") == 0){ return Seven; }
    if(strcmp(oneInput, "Eight") == 0){ return Eight; }
    if(strcmp(oneInput, "Nine") == 0){ return Nine; }
    if(strcmp(oneInput, "Ten") == 0){ return Ten; }
    if(strcmp(oneInput, "Jack") == 0){ return Jack; }
    if(strcmp(oneInput, "Queen") == 0){ return Queen; }
    if(strcmp(oneInput, "King") == 0){ return King; }
    if(strcmp(oneInput, "Ace") == 0){ return Ace; }
    if(strcmp(oneInput, "Ant") == 0){ return Anteater; }

    // Testing suits
    if(strcmp(oneInput, "H") == 0){ return Hearts; }
    if(strcmp(oneInput, "C") == 0){ return Clubs; }
    if(strcmp(oneInput, "D") == 0){ return Diamonds; }
    if(strcmp(oneInput, "S") == 0){ return Spades; }
    if(strcmp(oneInput, "Ants") == 0){ return Anteaters; }

    // Wrapup:  default
    printf("ERROR:  Value or suit not found.\n");
    return -1;
}

PILES setupTestGameDeal(PILE reference){
    // Setting up return value
    PILES answer;

    int numPlayers;
    printf("==========TESTING MODE:  GAME DEAL==========\n");
    printf("Number of players:  ");
    scanf(" %d", &numPlayers); // assuming valid number is put in, since this is for testing purposes only

    // Setting up return value (cont'd)
    answer.resize(numPlayers+2);

    /* Setting up player hands*/
    char val1[10], suit1[10], val2[10], suit2[10];
    int val1Num, suit1Num, val2Num, suit2Num;
    for(int i=0; i<numPlayers; i++){
        // Gathering/parsing input
        printf("Player %d's hand:  ", i+1);
        scanf("%s %s %s %s", val1, suit1, val2, suit2); // assuming valid numbers are put in, since this is for testing purposes only

        val1Num = parseTestInput(val1);
        suit1Num = parseTestInput(suit1);
        val2Num = parseTestInput(val2);
        suit2Num = parseTestInput(suit2);

        // Putting the cards in
        answer[i].emplace_back(val1Num, suit1Num);
        answer[i].emplace_back(val2Num, suit2Num);
    }

    /* Setting up the board */
    for(int i=0; i<5; i++){
        // Gathering/parsing input
        printf("Card %d on the board:  ", i+1);
        scanf("%s %s", val1, suit1);
        val1Num = parseTestInput(val1);
        suit1Num = parseTestInput(suit1);

        // Putting the card in
        answer[numPlayers].emplace_back(val1Num, suit1Num);

    }

    /* Setting up the leftover pile */
    int foundCard = 0;
    Card target;
    for(unsigned int k=0; k<reference.size(); k++){ // looping through to see if each card in the reference deck is in the player hands/community cards or not
        // Loop Setup
        foundCard = 0;
        target = reference[k];
        // Seeing if a given card is already used
        for(int l=0; l<numPlayers+1; l++){ // looping through player hands/community cards
            for(unsigned int m=0; m<answer[l].size(); m++){ // looping through each card in a player hand/in the community cards
                if(answer[l][m].getValue() == target.getValue() && answer[l][m].getSuit() == target.getSuit() ){
                    foundCard = 1;
                }
            }
        }
        // If a given card is not already used, put it in the remainder leftover pile
        if(!foundCard){
            answer[numPlayers+1].push_back(target);
        }
    }


    // Wrapup
    return answer;
}


/* MAIN FUNCTION */
int main(void){
    PILE deck = initDeck();
    printCard(deck[0]);
    printDeck(deck);

    PILES piles = setupTestGameDeal(deck);
    printPiles(piles);

    // Wrapup
    return 0;
}