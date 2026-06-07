/* cards.cpp */

#include "cards.hpp"

PILE initDeck(void){
    PILE deck;
    deck.reserve(54);

    //Card newCard;

    // Standard deck cards
    for(int suit = 0; suit<4; suit++){
        for(int val=0; val<13; val++){
            //newCard = Card(val, suit);
            deck.emplace_back(val, suit); // creates new Card and puts it in deck
        }
    }

    // Anteater cards
    deck.emplace_back(Anteater, Anteaters);
    deck.emplace_back(Anteater, Anteaters);

    return deck;
}

void printCard(Card card){
    char val[3];
    const char *suit;
    switch(card.getSuit()){
        case Hearts:  {suit = "♥"; break;}
        case Clubs:  {suit = "♣"; break;}
        case Diamonds:  {suit = "♦"; break;}
        case Spades:  {suit = "♠"; break;}
        case Anteaters:  {suit = "t"; break;}
        default:  {suit = "?"; break;}
    };
    switch(card.getValue()){
        case Two:
        case Three:
        case Four:
        case Five:
        case Six:
        case Seven:
        case Eight:  
        case Nine:  {sprintf( val, "%c ", (char)(card.getValue()+2+'0') ); break;}
        case Ten:  {sprintf(val, "10"); break;}
        case Jack:  {sprintf(val, "J "); break;}
        case Queen:  {sprintf(val, "Q "); break;}
        case King:  {sprintf(val, "K "); break;}
        case Ace:  {sprintf(val, "A "); break;}
        case Anteater:  {sprintf(val, "An"); break;}
        default:  {sprintf(val, "zz"); break;}
    }

    // Wrapup
    printf("Card:  %s%s\n", val, suit);
}

int printDeck(PILE deck){
    char val[3];
    const char *suit;
    for(unsigned int i=0; i<deck.size(); i++){ // adding unsigned to get rid of a warning from the compiler
        switch(deck[i].getSuit()){
            case Hearts:  {suit = "♥"; break;}
            case Clubs:  {suit = "♣"; break;}
            case Diamonds:  {suit = "♦"; break;}
            case Spades:  {suit = "♠"; break;}
            case Anteaters:  {suit = "t"; break;}
            default:  {suit = "?"; break;}
        };
        switch(deck[i].getValue()){
            case Two:
            case Three:
            case Four:
            case Five:
            case Six:
            case Seven:
            case Eight:  
            case Nine:  {sprintf( val, "%c ", (char)(deck[i].getValue()+2+'0') ); break;}
            case Ten:  {sprintf(val, "10"); break;}
            case Jack:  {sprintf(val, "J "); break;}
            case Queen:  {sprintf(val, "Q "); break;}
            case King:  {sprintf(val, "K "); break;}
            case Ace:  {sprintf(val, "A "); break;}
            case Anteater:  {sprintf(val, "An"); break;}
            default:  {sprintf(val, "zz"); break;}
        }

        printf("Card %d:  %s%s\t\t", i+1, val, suit);
    }
    printf("\n");
    return 0;
}

int printPiles(PILES piles){
    int n = piles.size(); // number of entries in piles -- all but the last two used for players, the penultimate one used for community cards, and the last one used for remaining leftover cards
    if(piles.size() < 3){
        printf("ERROR:  There must be at least one player hand pile, one board pile, and one leftover cards pile.\n");
        return -1;
    }

    /* Printing player hands */
    printf("==========Player hands==========\n");
    for(int i=0; i<n-2; i++){
        printf("Player %d's hand:  ", i+1);
        printDeck(piles[i]);
    }
    printf("\n");

    /* Printing community cards */
    printf("==========Community cards==========\n");
    printDeck(piles[n-2]);
    printf("\n");
    
    /* Printing the rest of the deck */
    printf("==========The rest of the deck==========\n");
    printDeck(piles[n-1]);
    printf("\n");

    // Wrapup
    return 0;
}