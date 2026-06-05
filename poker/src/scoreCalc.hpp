/* scoreCalc.hpp */

#ifndef SCORE_CALC
#define SCORE_CALC

#include "cards.hpp"
#include "gamestate.hpp"

#define ROYALFLUSH_SCORE 10000
#define STRAIGHTFLUSH_SCORE 9000
#define FOURKIND_SCORE 8000

#define HIGHCARD_RANK       0x0
#define ONEPAIR_RANK        0x1
#define TWOPAIR_RANK        0x2
#define THREEKIND_RANK      0x3
#define STRAIGHT_RANK       0x4
#define FLUSH_RANK          0x5
#define FULLHOUSE_RANK      0x6
#define FOURKIND_RANK       0x7
#define ANTEATERPAIR_RANK   0x8
#define STRAIGHTFLUSH_RANK  0x9
#define ROYALFLUSH_RANK     0xA

// int playerScore(PILES piles, int playerNum);
int scoreHand(PILES piles, int playerNum);
// NOTE:  All of these return 0 if the ranked hand type doesn't appear and returns a score value (score + kicker(s)) if it does
int isRoyalFlush(PILES piles, int playerNum);
int isStraightFlush(PILES piles, int playerNum);
int isAnteaterPair(PILES piles, int playerNum);
int isFourKind(PILES piles, int playerNum);
int isFullHouse(PILES piles, int playerNum);
int isFlush(PILES piles, int playerNum);
int isStraight(PILES piles, int playerNum);
int isThreeKind(PILES piles, int playerNum);
int isTwoPair(PILES piles, int playerNum);
int isOnePair(PILES piles, int playerNum);
int isHighCard(PILES piles, int playerNum);
int val_translator(int x);
int makeScore(int hand, int max, int second, int third, int fourth, int fifth);

#endif
