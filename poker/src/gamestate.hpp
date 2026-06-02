/* gamestate.hpp */

#ifndef GAMESTATE_H
#define GAMESTATE_H

#include "cards.hpp"

PILE oneShuffle(PILE deck);
PILE shuffle(PILE deck);
PILES deal(PILE deck, int numPlayers, int dealerPlayer);


#endif