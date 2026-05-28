/* clientGUI.hpp */

#ifndef CLIENTGUI
#define CLIENTGUI

#include "cards.hpp"
#include "data.hpp"

// NEW/UPDATED FUNCTIONS
    /* textMenu():  Based on information from the server regarding login stuff, shows a text menu; player inputs then impact the login info; new LOGININFO is returned */
    LOGININFO textMenu(LOGININFO serverLoginInfo);

    /* textMenu():  Based on information from the server regarding login stuff, shows a text menu; player inputs then impact the login info; new LOGININFO is returned */
    LOGININFO guiMenu(LOGININFO serverLoginInfo);

    /* printLogMessage():  Used to notify the player of what is going on the game by keeping track of things in a log; will notify players of bets/folds, who wins the hand, start/end of a round, etc. */
    void printLogMessage(const char *msg);
// end new/updated functions

/* displayWaitingScreen():  Displays the waiting screen while waiting for other players to join */
void displayWaitingScreen(int numPlayers);

/* displayCards():  Displays the player's hole cards and the the community cards*/
void displayCards(PILES piles);

/* displayPot():  Displays the pot */
void displayPot(int pot);

/* displayScores():  Displays player scores */
void displayScores(PLAYERS players);

/* askAction():  Gets a bet or special bet (call or fold) from player, if applicable, based on the current state of the game and which player they are */
int askAction(GAMESTATE gameState, int playerNum);

/* displayAnalysis():  ???*/
int displayAnalysis(PILES piles, int playerNum);


#endif
