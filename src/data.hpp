/* data.hpp */

#ifndef DATA_H
#define DATA_H

#include <vector>
#include <string>
#include "cards.hpp"

#define START_POINTS 100


typedef std::string String;

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

    struct Player{
        int playerNum{-1}; // numbering starts at 0
        char name[41]{"Unnammed player"}; // holds the player's name
        int playerType{-1}; // holds if player is Human or Computer (see PlayerType enum)
        int score{START_POINTS}; // holds a player's score (i.e. the amount of points they currently have); does not include points that have been bet during the hand
        int bet{0}; // holds a player's bet for the current round of betting (i.e. reset to 0 at the end of each round)
        int isInHand{1}; // 1 if player is still playing in the hand (i.e. hasn't folded), and 0 if not; reset at the beginning of each hand
        int isEliminated{0}; // should be removed from the list if this is true (1); indicates whether a player has 0 points left at the end of a round
        int playerSocket{-1}; // holds the player's (a client) socket number
        int total_bet{0};

        void PrintPlayer(){
            printf("Player:  \n");
            printf("Name:  %s\n", name);
            printf("Player type:  %s\n", (playerType == Human) ? "Human" : (playerType == Computer) ? "Computer" : "unrecognized");
            printf("Score:  %d points\n", score);
            printf("Current bet for the round:  %d points\n", bet);
            printf("Still in the hand?: %s\n", (isInHand == 1) ? "yes" : "no");
            printf("Still in the game (i.e. not eliminated):  %s\n", (isEliminated == 0) ? "yes" : "no");
            printf("Player socket number:  %d\n\n", playerSocket);
        }
    };
    typedef struct Player PLAYER;
    typedef std::vector<PLAYER> PLAYERS;

    typedef std::vector<int> PLAYERSFOUND; // 0 if player is not found; 1 or 2 if player is Human / Computer (see PlayerType enum)
    struct LoginInfo{
        int packageType = login; // used for sending data via the buffer between the client and the server (see PackageType enum)
        int isHost{0}; // represents if the player is the "host" player (the host determines the number of players to enter the game, the number of bots, etc.)
        char playerName[41]{"No name"}; // holds the player's name
        int playerNum{-1}; // represents the player number at the table (numbering starts at 0)
        char password[41]{"No password"}; // set by the player if the host (from client to server), or entered by a joining player if not the host (from server to client)
        int playerType{-1}; // Human or Computer (see PlayerType enum)
        int numPlayers{0}; // number of players in the room
        PLAYERSFOUND playersFound;

        void PrintLoginInfo(void){
            printf("Login information:  \n");
            printf("Package type:  %s\n", (packageType == login) ? "login" : "incorrect");
            printf("Is host?:  %s\n", (isHost == 1) ? "yes" : "no");
            printf("Player name:  %s\n", playerName);
            printf("Room password:  %s\n", password);
            printf("Player type:  %s\n", (playerType == Human) ? "Human" : (playerType == Computer) ? "Computer" : "unrecognized");
            //printf("Players found:  Sorry, not testing that right now!\n\n");
            printf("\n");
        }
        void hostUpdate(){
            numPlayers++;
        }
        void nonHostUpdate(){
            numPlayers++;
        }

    };
    typedef struct LoginInfo LOGININFO;

    struct GameState{
        int packageType = game; // for serializing/deserializing structures purposes
        int numPlayers{0}; // total number of players in the game
        int round{Preflop}; // holds round number (see Round enum)
        int dealerPlayer{0}; // represents which player is the dealer for the hand (note that the server does the dealing, not the player)
        int pot{0}; // amount of money in the pot
        int callAmount{0}; // the amount of points currently needed to call in a round
        int playerTurn{0}; // holds which player's turn it is
        int greatest{0}; // holds the greatest bet (ADDED BETA MAY30)
        PILES allCards; // holds all the cards in the hand (including "leftover" cards)
        PLAYERS players; // holds player information
        

        void PrintGameState(void){
            printf("Game state information:  \n");
            printf("Package type:  %s\n", (packageType == game) ? "game" : "incorrect");
            printf("Number of players:  %d\n", numPlayers);
            printf("Round number (refer to enum Round for details):  %d\n", round);
            printf("Dealer (player number):  %d\n", dealerPlayer);
            printf("Cards:  Use printPiles(), please!\n");
            printf("Pot:  %d\n", pot);
            printf("Call amount:  %d\n", callAmount);
            printf("Player information:  Sorry, not supporting that right now!  Try the PrintPlayer() function instead\n");
            printf("Turn:  Player %d\n\n", playerTurn);
        }

    };
    /* GameState contents:  packageType, numPlayer, round, dealerPlayer, allCards, pot, callAmount, players, playerTurn */
    typedef struct GameState GAMESTATE;

/* end structures and vectors*/

#endif