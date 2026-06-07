/* cards.hpp */

/* AAA:  Change getVal() to getValue() */

#ifndef CARDS_H
#define CARDS_H


#include <stdio.h>
#include <vector>


enum val{
    Two,
    Three,
    Four,
    Five,
    Six,
    Seven,
    Eight,
    Nine,
    Ten,
    Jack,
    Queen,
    King,
    Ace,
    Anteater // not in CardPractice version
};

enum suit{
    Hearts,
    Clubs,
    Diamonds,
    Spades,
    Anteaters // not in CardPractice version
};

class Card{
public:
    int val;
    int suit;

    Card(void){ // overloading
        val = -1;
        suit = -1;
    }
    Card(int valVar, int suitVar){
        val = valVar;
        suit = suitVar;
    }
    ~Card(){
    } // destructor

    int getValue(void) const { // const means funct can't modify anything--good for all getters
        return val;
    }
    int getSuit(void) const{
        return suit;
    }
    
};

typedef std::vector<Card> PILE;

typedef std::vector<PILE> PILES;


/* initDeck():  initializes a deck of cards and returns a deck of cards (PILE) */
PILE initDeck(void);

/* printCard():  prints a card*/
void printCard(Card card);

/* printDeck():  Can be used to print any PILE (hole cards, community cards, deck, etc.)*/
int printDeck(PILE deck);

/* printPiles():  Used to print all hole cards, community cards, and leftover cards (in that order) */
int printPiles(PILES piles);


#endif