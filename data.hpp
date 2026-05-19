/* data.hpp */

#ifndef DATA_H
#define DATA_H

#include <vector>
#include "cards.hpp"

/* Enumerators */
    enum Round{
        Preflop,
        Flop,
        Turn, 
        River,
        Showdown
    };

    enum PlayerType{
        Human = 1,
        Computer = 2
    };

    enum SpecialBets{
        Check = 0,
        Fold = -1
    };

    enum PackageType{
        login,
        game
    };
/* end enumerators */


/* Structures and relevant Vectors */
    typedef std::vector<unsigned char> BUF; // used to represent the buffer for TCP/IP communication

    struct GameState{
        int packageType = game; // for serializing/deserializing structures purposes
        int numPlayers; // total number of players in the game
        int round; // holds round number (see Round enum)
        int dealerPlayer; // represents which player is the dealer for the hand (note that the server does the dealing, not the player)
        PILES allCards; // holds all the cards in the hand (including "leftover" cards)
        int pot; // amount of money in the pot
        int callAmount; // the amount of points currently needed to call in a round
        int playerTurn; // holds which player's turn it is
    };
    typedef struct GameState GAMESTATE;

    struct Player{
        int playerNum; // numbering starts at 0
        char *name; // holds the player's name
        int playerType; // holds if player is Human or Computer (see PlayerType enum)
        int score; // holds a player's score (i.e. the amount of points they currently have); does not include points that have been bet during the hand
        int bet; // holds a player's bet for the current round of betting (i.e. reset to 0 at the end of each round)
        int isInHand; // 1 if player is still playing in the hand (i.e. hasn't folded), and 0 if not; reset at the beginning of each hand
        int isEliminated; // should be removed from the list if this is true (1); indicates whether a player has 0 points left at the end of a round
        int playerSocket; // holds the player's (a client) socket number
    };
    typedef struct Player PLAYER;
    typedef std::vector<PLAYER> PLAYERS;

    typedef std::vector<int> PLAYERSFOUND; // 0 if player is not found; 1 or 2 if player is Human / Computer (see PlayerType enum)
    struct LoginInfo{
        int packageType = login; // used for sending data via the buffer between the client and the server (see PackageType enum)
        int isHost; // represents if the player is the "host" player (the host determines the number of players to enter the game, the number of bots, etc.)
        char *playerName; // holds the player's name
        int playerNum; // represents the player number at the table (numbering starts at 0)
        char *password; // set by the player if the host (from client to server), or entered by a joining player if not the host (from server to client)
        int playerType; // Human or Computer (see PlayerType enum)
        int numPlayers; // number of players in the room
        PLAYERSFOUND playersFound;

        void PrintLoginInfo(void){
            printf("Package type:  %s\n", (packageType == login) ? "login" : "incorrect");
            printf("Is host?:  %s/n", (packageType));
        }
    };
    typedef struct LoginInfo LOGININFO;

/* end structures and vectors*/

#endif