// What I need to do:  

// Dealing function (update)

// Player input legality functions (different file)

#include <vector>
#include <stdio.h>
#include <string.h>
#include <stdlib.h> // used for rand() and srand() functions
#include <time.h> // used for srand() arg specification

#include "cards.hpp"
#include "data.hpp"
#include "gamestate.hpp"
// CODEX FIX: showdown winner selection uses scoreHand() from the added scoreCalc files.
#include "scoreCalc.hpp"

// CODEX FIX: find the next active player without landing on folded/eliminated seats.
static int nextActivePlayer(const GAMESTATE& gameState, int fromPlayer){
    if(gameState.players.empty()){
        return -1;
    }

    int current = fromPlayer;
    for(unsigned int i=0; i<gameState.players.size(); i++){
        current = (current == gameState.numPlayers - 1) ? 0 : current + 1;
        if(gameState.players[current].isInHand && !gameState.players[current].isEliminated){
            return current;
        }
    }

    return -1;
}

// CODEX FIX: find the previous active player for the "last actor" marker on a betting street.
static int previousActivePlayer(const GAMESTATE& gameState, int fromPlayer){
    if(gameState.players.empty()){
        return -1;
    }

    int current = fromPlayer;
    for(unsigned int i=0; i<gameState.players.size(); i++){
        current = (current == 0) ? gameState.numPlayers - 1 : current - 1;
        if(gameState.players[current].isInHand && !gameState.players[current].isEliminated){
            return current;
        }
    }

    return -1;
}

// CODEX FIX: every active player must have matched the call amount before a betting round can end.
static int activeBetsAreMatched(const GAMESTATE& gameState){
    for(unsigned int i=0; i<gameState.players.size(); i++){
        if(gameState.players[i].isInHand && !gameState.players[i].isEliminated
           && gameState.players[i].bet != gameState.callAmount){
            return 0;
        }
    }

    return 1;
}

// CODEX FIX: reset bets and choose the correct first/last actors for postflop betting rounds.
static GAMESTATE beginPostflopBettingRound(GAMESTATE gameState, int nextRound){
    GAMESTATE answer = gameState;

    for(unsigned int i=0; i<answer.players.size(); i++){
        answer.players[i].bet = 0;
    }
    answer.callAmount = 0;
    answer.round = nextRound;
    // CODEX FIX: postflop action starts left of the dealer.
    answer.playerTurn = nextActivePlayer(answer, answer.dealerPlayer);
    // CODEX FIX: the previous active seat before first action is last to act on this street.
    answer.greatest = previousActivePlayer(answer, answer.playerTurn);

    return answer;
}

// CODEX FIX: pause a completed hand so clients can see the final state until the dealer starts the next hand.
static GAMESTATE pauseHandForDealerStart(GAMESTATE gameState){
    GAMESTATE answer = gameState;

    for(unsigned int i=0; i<answer.players.size(); i++){
        // CODEX FIX: completed hands should not leave old street bets visible/actionable.
        answer.players[i].bet = 0;
    }

    answer.callAmount = 0; // CODEX FIX: no one has anything to call while waiting for the dealer.
    answer.playerTurn = -1; // CODEX FIX: -1 marks "between hands" so no player action buttons enable.
    answer.greatest = -1; // CODEX FIX: no betting loop is open while the dealer start button is waiting.

    return answer;
}


// My functions
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

    PILE shuffle(PILE deck){ // not checked yet
        PILE answer = deck;
        for(int i=0; i<7; i++){ // seven shuffles
            answer = oneShuffle(answer);
        }
        return answer;
    }


    PILES deal(PILE deck, int numPlayers, int dealerPlayer){ // not checked yet
        // List of piles that will be returned
        PILES answer;

        // DEBUGGING
        printDeck(deck);


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

        int d = (dealerPlayer == numPlayers-1) ? 0 : dealerPlayer + 1;
        for(int i=0; i<2; i++){ // each player gets two cards
            for(int j=0; j<numPlayers; j++){ // looping through players
                // printf("Player receiving card:  %d\n", d);
                // printf("Card number player receives:  %d\n", i*numPlayers + j);
                // printf("Card is:  %d %d\n", deck[i*numPlayers+j].getValue(), deck[i*numPlayers+j].getSuit());
                cardNum = i*numPlayers + j;
                answer[d].push_back(deck[cardNum]);
                //answer[j].push_back(deck[cardNum]); // adds card to player hand
                d = (d == numPlayers-1) ? 0 : d + 1;
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

// Sam and David's functions



GAMESTATE updateGameState(GAMESTATE recvGameState){ // clients only ever modify pot, call amount, player.isInHand, player.bet, player.score, and player.greatest; they do not modify the round number or player turn
    GAMESTATE answer = recvGameState;
    answer.numPlayers = answer.players.size();
    // CODEX FIX: remember who just acted before advancing playerTurn.
    int actingPlayer = answer.playerTurn;

    // Checking there is more than one player left in the round
    int numLeft = 0;
    for(unsigned int i=0; i<answer.players.size(); i++){
        if(answer.players[i].isInHand){
            numLeft++;
        }
    }
    if(numLeft == 0){
        printf("ERROR:  There are no players left in the round.\n");
        return answer;
    }
    if(numLeft == 1){ // one player left in the round -- give them the pot points and wait for the dealer
        printf("Only one player left in the round!  They win the pot!\n");
        for(unsigned int i=0; i<answer.players.size(); i++){
            if(answer.players[i].isInHand){
                answer.players[i].score += answer.pot; // player wins the pot
                answer.pot = 0; // CODEX FIX: the fold winner receives the full pot before the next-hand pause.
                // CODEX FIX: do not auto-deal; pause so the dealer can press START HAND.
                answer = pauseHandForDealerStart(answer);
                return answer;
            }
        }

    }

    // CODEX FIX: finish the betting round only after the last required actor has acted and bets match.
    if(actingPlayer == answer.greatest && activeBetsAreMatched(answer)){
        if(answer.round == Preflop){ answer = flopUpdate(answer); }
        else if(answer.round == Flop){ answer = turnUpdate(answer); }
        else if(answer.round == Turn){ answer = riverUpdate(answer); }
        else if(answer.round == River){ answer = showdownUpdate(answer); } // CODEX FIX: leave showdown visible until the dealer starts the next hand.
        return answer;
    }

    // CODEX FIX: otherwise advance to the next active player and keep the same betting round.
    answer.playerTurn = nextActivePlayer(answer, answer.playerTurn);
    if(answer.playerTurn < 0){
        // CODEX FIX: if no next active player exists, leave the state unchanged instead of indexing badly.
        answer.playerTurn = actingPlayer;
    }

    return answer;
}



GAMESTATE startRound(GAMESTATE gameState){ // NOTE:  startRound() calls endRoundChecks() at the beginning -- endRoundChecks() should NOT be called directly except through this function
    GAMESTATE answer = endRoundChecks(gameState);
    answer.numPlayers = answer.players.size(); // just in case

    if(answer.numPlayers == 0){
        printf("Hmmm...there are no players left!\n");
        return answer;
    }
    if(answer.numPlayers == 1){
        printf("One player is left, and they have won the game!");
        return answer;
    }

    answer.pot = 0;
    answer.callAmount = 0;
    answer.dealerPlayer = (answer.dealerPlayer == answer.numPlayers-1) ? 0 : answer.dealerPlayer + 1;
    
    for(unsigned int i=0; i<answer.players.size(); i++){
        answer.players[i].bet = 0;
        answer.players[i].isInHand = 1;
    }

    
    // Card distribution
    PILE deck = initDeck();
    deck = shuffle(deck);
    answer.allCards = deal(deck, answer.numPlayers, answer.dealerPlayer);

    // Setting up for preflop
    answer.round = Preflop;
    answer = preflopUpdate(answer);

    // Wrapup
    return answer;
}

// ZZZ:  HOUSE RULE:  If relevant players do not have enough money to cover the small and big blinds, they are out
GAMESTATE endRoundChecks(GAMESTATE gameState){ // determines hypothetetical "previous" dealer, if necessary
    GAMESTATE answer = gameState;
    answer.numPlayers = answer.players.size(); // just in case

    if(answer.players.size() <= 0){
        printf("ERROR:  There should be at least one player left in the game!  Can't eliminate players if there are none...\n");
        return answer;
    }
    if(answer.players.size() == 1){
        printf("Uh...there's already only one player left.  They should have won by now.\n");
        return answer;
    }

    for(int i=answer.players.size()-1; i >= 0; i--){ // have to loop backwards to avoid removal issues
        if(answer.players[i].score <= 0 ){ // player doesn't have enough points at end of round and should be eliminated
            if(i == answer.dealerPlayer){
                answer.dealerPlayer = (answer.dealerPlayer == 0) ? answer.numPlayers-2 : answer.dealerPlayer - 1; // "previous" dealer player
            }
            if(i < answer.dealerPlayer){ // player before dealer was eliminated
                answer.dealerPlayer--;
            }
            answer.players[i].isEliminated = 1;
            answer.players.erase(answer.players.begin() + i);
            answer.numPlayers--;
        }
    }
    if(answer.numPlayers == 1){
        printf("Only one player has points left, so they win!\n");
        return answer;
    }

    // Removing players who can't cover their blinds
    int smallBlindPlayer = (answer.dealerPlayer == answer.numPlayers-1) ? 0 : answer.dealerPlayer + 1;
    int bigBlindPlayer = (smallBlindPlayer == answer.numPlayers-1) ? 0 : smallBlindPlayer + 1;

    if(answer.numPlayers == 2){
        if(answer.players[0].score < 2){
            printf("Player 1 is eliminated!  Player 2 wins!\n");
            answer.players[0].isEliminated = 1;
            answer.players.erase(answer.players.begin() + 0);
            answer.numPlayers--;
        } else if(answer.players[1].score < 2){
            printf("Player 2 is eliminated!  Player 1 wins!\n");
            answer.players[1].isEliminated = 1;
            answer.players.erase(answer.players.begin() + 1);
            answer.numPlayers--;
        }
    }
    while(answer.numPlayers > 2 && answer.players[bigBlindPlayer].score < 2){
        printf("Player %d does not have enough money to cover the big blind and has been eliminated.\n", bigBlindPlayer);
        answer.players[bigBlindPlayer].isEliminated = 1;
        answer.players.erase(answer.players.begin() + bigBlindPlayer);
        answer.numPlayers--;
        if(bigBlindPlayer < answer.dealerPlayer){
            answer.dealerPlayer--;
        }
        bigBlindPlayer = (bigBlindPlayer == answer.numPlayers-1) ? 0 : bigBlindPlayer + 1;
    }
    if(answer.numPlayers == 2){
        if(answer.players[0].score < 2){
            printf("Player 1 is eliminated!  Player 2 wins!\n");
            answer.players[0].isEliminated = 1;
            answer.players.erase(answer.players.begin() + 0);
            answer.numPlayers--;
        } else if(answer.players[1].score < 2){
            printf("Player 2 is eliminated!  Player 1 wins!\n");
            answer.players[1].isEliminated = 1;
            answer.players.erase(answer.players.begin() + 1);
            answer.numPlayers--;
        }
    }
    

    // Updating internal player numbers
    for(unsigned int i=0; i<answer.players.size(); i++){
        answer.players[i].playerNum = i;
    }

    // Wrapup
    return answer;
}


GAMESTATE preflopUpdate(GAMESTATE gameState){
    GAMESTATE answer = gameState;

    // Handling the pot
    int smallBlindPlayer = (answer.dealerPlayer == answer.numPlayers-1) ? 0 : answer.dealerPlayer + 1;
    int bigBlindPlayer = (smallBlindPlayer == answer.numPlayers-1) ? 0 : smallBlindPlayer + 1;
    answer.pot += 3;
    answer.players[smallBlindPlayer].score -= 1;
    answer.players[bigBlindPlayer].score -= 2;

    // Handling player stats
    answer.players[smallBlindPlayer].bet = 1;
    answer.players[bigBlindPlayer].bet = 2;
    answer.greatest = bigBlindPlayer;
    answer.callAmount = 2;
    answer.playerTurn = (bigBlindPlayer == answer.numPlayers-1) ? 0 : bigBlindPlayer + 1;

    // Handling round
    answer.round = Preflop;


    // Wrapup
    return answer;
}

GAMESTATE flopUpdate(GAMESTATE gameState){
    // CODEX FIX: centralize postflop setup so bets/turn order reset correctly.
    GAMESTATE answer = beginPostflopBettingRound(gameState, Flop);

    // Wrapup
    return answer;
}

GAMESTATE turnUpdate(GAMESTATE gameState){
    // CODEX FIX: centralize postflop setup so bets/turn order reset correctly.
    GAMESTATE answer = beginPostflopBettingRound(gameState, Turn);

    // Wrapup
    return answer;
}

GAMESTATE riverUpdate(GAMESTATE gameState){
    // CODEX FIX: centralize postflop setup and actually mark the game as River.
    GAMESTATE answer = beginPostflopBettingRound(gameState, River);

    // Wrapup
    return answer;
}
GAMESTATE showdownUpdate(GAMESTATE gameState){
    GAMESTATE answer = gameState;

    printf("\n\n====================SHOWDOWN====================\n");
    printPiles(answer.allCards);
    printf("================================================\n\n");

    // CODEX FIX: mark this state as showdown before the next hand starts.
    answer.round = Showdown;
    // CODEX FIX: clear action state and pause so clients can see all five board cards at showdown.
    answer = pauseHandForDealerStart(answer);

    if(answer.allCards.size() < answer.players.size() + 2){
        // CODEX FIX: avoid scoring with malformed card piles.
        printf("ERROR:  Cannot resolve showdown because allCards does not contain player/community/deck piles.\n");
        return answer;
    }

    // CODEX FIX: store actual player indices, not positions in a filtered score vector.
    std::vector<int> winners; // CODEX FIX: use std::vector because this file does not import the std namespace.
    int bestScore = -1;

    for(unsigned int i=0; i<answer.players.size(); i++){
        if(answer.players[i].isInHand != 1 || answer.players[i].isEliminated){
            // CODEX FIX: folded/eliminated players cannot win the showdown pot.
            continue;
        }

        // CODEX FIX: scoreHand() evaluates this player's hand plus the community pile.
        int playerScore = scoreHand(answer.allCards, i);
        if(playerScore > bestScore){
            // CODEX FIX: new best hand replaces the previous winner list.
            bestScore = playerScore;
            winners.clear();
            winners.push_back((int)i);
        } else if(playerScore == bestScore){
            // CODEX FIX: equal scores split the main pot.
            winners.push_back((int)i);
        }
    }

    if(winners.empty()){
        // CODEX FIX: leave the pot untouched if no eligible winner was found.
        printf("ERROR:  Cannot resolve showdown because no active winner was found.\n");
        return answer;
    }

    // CODEX FIX: split the main pot evenly among tied winners.
    int splitAmount = answer.pot / (int)winners.size();
    // CODEX FIX: assign any odd remainder to the first winner so no chips disappear.
    int remainder = answer.pot % (int)winners.size();

    for(unsigned int i=0; i<winners.size(); i++){
        int award = splitAmount;
        if(i == 0){
            // CODEX FIX: preserve total chip count when the pot does not divide evenly.
            award += remainder;
        }
        answer.players[winners[i]].score += award;
    }

    // CODEX FIX: the whole main pot has been awarded.
    answer.pot = 0;

    return answer;
}
