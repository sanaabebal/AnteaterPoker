/* gamestate.hpp */

#ifndef GAMESTATE_H
#define GAMESTATE_H

#include "cards.hpp"

PILE oneShuffle(PILE deck);
PILE shuffle(PILE deck);
PILES deal(PILE deck, int numPlayers, int dealerPlayer);

GAMESTATE updateGameState(GAMESTATE recvGameState);

GAMESTATE startRound(GAMESTATE gameState);
GAMESTATE endRoundChecks(GAMESTATE gameState);

GAMESTATE preflopUpdate(GAMESTATE gameState);
GAMESTATE flopUpdate(GAMESTATE gameState);
GAMESTATE turnUpdate(GAMESTATE gameState);
GAMESTATE riverUpdate(GAMESTATE gameState);
GAMESTATE showdownUpdate(GAMESTATE gameState);


#endif