#include <iostream>
#include <unordered_map>
#include <stdio.h>
#include <vector>
#include <algorithm>
#include "scoreCalc.hpp"
#include "cards.hpp"
#include "data.hpp"
using namespace std;

// YYY:  FIX THIS!!!
int findKicker(PILES piles, int playerNum, int excludeVal, int firstHigh, int secondHigh) {
    int playerNumBase = playerNum;

    if(piles.size() < 3 || playerNumBase < 0 || (unsigned)playerNumBase >= piles.size() - 2) {
        printf("ERROR:  Can't determine if there is a kicker if there are not enough piles or the wrong player number was provided.\n");
        return -1;
    }

    // Vars
    PILE hand = piles[playerNumBase];
    PILE comm = piles[piles.size() - 2];

    int highest = 0;
    int testVal;

    // Checking hole cards
    for (Card x : hand) {
        testVal = x.getValue();

        if(testVal != excludeVal &&
            testVal != firstHigh &&
            testVal != secondHigh &&
            testVal > highest) {
                highest = testVal;
            }
    }

    // Checking Community cards
    for (Card x : comm) {
        testVal = x.getValue();

        if(testVal != excludeVal &&
            testVal != firstHigh &&
            testVal != secondHigh &&
            testVal > highest) {
                highest = testVal;
            }
    }
    return highest;
}

#ifdef TESTING
// findKicker():  Helper function that returns the value of the highest card the player has access to
int findKicker(PILES piles, int playerNum, int excludeVal, int firstHigh, int secondHigh) { // first and second high used when second or third kickers need to be found; otherwise, they are set to -1
    int playerNumBase = playerNum; // in case there is a change/miscommunication about the purpose of player num (assuming player numbering starts at 0)
    // Error checking
    if(piles.size() < 3 || playerNumBase < 0 || (unsigned)playerNumBase >= piles.size() - 2) {
        printf("ERROR:  Can't determine if there is a kicker if there are not enough piles or the wrong player number was provided.\n");
        return -1;
    }

    // Vars
    PILE hand = piles[playerNumBase];
    PILE comm = piles[piles.size() - 2];

    int highest = Two;
    int countFirstHigh = 0;
    int countSecondHigh = 0;
    
    // Checking hole cards
    int testVal;
    for(unsigned int i = 0; i < hand.size(); i++) {
        testVal = hand[i].getValue();
        if(testVal > highest && testVal != firstHigh && testVal != secondHigh) {
            highest = testVal;
        } else if(testVal == firstHigh && countFirstHigh == 0) { //
            countFirstHigh == 
        }

    }

    
}
#endif


int isRoyalFlush(PILES piles, int playerNum) {
    int playerNumBase = playerNum; // in case there is a change/miscommunication about the purpose of player num (assuming player numbering starts at 0)
    // Error checking
    if(piles.size() < 3 || playerNumBase < 0 || (unsigned)playerNumBase >= piles.size() - 2) {
        printf("ERROR:  Can't determine if there is a royal flush if there are not enough piles or the wrong player number was provided.\n");
        return -1;
    }

    // Vars
    PILE hand = piles[playerNumBase];
    PILE comm = piles[piles.size() - 2];

    // Creating card found array
    int cardFound[4][5];
    for(int i = 0; i < 4; i++ ){ // i refers to the suit (in order:  hearts, clubs, diamonds, spades)
        for(int j = 0; j < 5; j++) { // j refers to the value (in order:  ten, jack, queen, king, ace)
            cardFound[i][j] = 0;
        }
    }

    // Checking player hand
    Card card;
    int valIndex = -1;
    for(unsigned int i = 0; i < hand.size(); i++) {
        card = hand[i];
        if(card.getValue() >= Ten && card.getValue() <= Ace) {
            valIndex = card.getValue() - Ten; // Ten is the 0th index; Ace is the 4th index
            if(card.getSuit() >= Hearts && card.getSuit() <= Spades) {
                cardFound[card.getSuit()][valIndex] = 1;
            } 
        }
    }

    // Checking community hand
    for(unsigned int i = 0; i < comm.size(); i++) {
        card = comm[i];
        if(card.getValue() >= Ten && card.getValue() <= Ace) {
            valIndex = card.getValue() - Ten; // Ten is the 0th index; Ace is the 4th index
            if(card.getSuit() >= Hearts && card.getSuit() <= Spades) {
                cardFound[card.getSuit()][valIndex] = 1;
            } 
        }
    }

    // Checking to see if all of the indices of one suit were found
    int sum = 0;
    for(int suit = 0; suit < 4; suit++) {
        sum = 0;
        for(int val = 0; val < 5; val++) {
            sum += cardFound[suit][val];
        }
        if(sum == 5) {
            return makeScore(ROYALFLUSH_RANK, 0, 0, 0, 0, 0);
        }
    }
    return 0;
}

int isStraightFlush(PILES piles, int playerNum) { // returns straight flush score + high value if a straight flush is present
    int playerNumBase = playerNum; // in case there is a change/miscommunication about the purpose of player num (assuming player numbering starts at 0)
    // Error checking
    if(piles.size() < 3 || playerNumBase < 0 || (unsigned)playerNumBase >= piles.size() - 2) {
        printf("ERROR:  Can't determine if there is a straight flush if there are not enough piles or the wrong player number was provided.\n");
        return -1;
    }

    // Vars
    PILE hand = piles[playerNumBase];
    PILE comm = piles[piles.size() - 2];
    int foundCards[4][15] = {0}; // ZZZ:  Hopefully this works as intended?
    // not sure about the Ace value, might need size 15 bc of Ace which usually 14
     // Checking player hand
    Card card;
    for(unsigned int i = 0; i < hand.size(); i++) {
        card = hand[i];
        if(card.getValue() >= Two && card.getValue() <= Ace) {
            if(card.getSuit() >= Hearts && card.getSuit() <= Spades) {
                foundCards[card.getSuit()][card.getValue()] = 1;
            } 
        }
    }

    // Checking community hand
    for(unsigned int i = 0; i < comm.size(); i++) {
        card = comm[i];
        if(card.getValue() >= Two && card.getValue() <= Ace) {
            if(card.getSuit() >= Hearts && card.getSuit() <= Spades) {
                foundCards[card.getSuit()][card.getValue()] = 1;
            } 
        }
    }
    
    // Checking to see if there are five consecutive cards of one suit found
    int sum = 0;
    for(int suit = 0; suit < 4; suit++) {
        sum = 0;
        for(int val = Ace; val >= Two; val--) { // have to go backwards to find the highest-ranking hand
            if(foundCards[suit][val] == 1) {
                sum++;
            } 
            else{ sum = 0; }

            // if(sum == 5){ return STRAIGHTFLUSH_SCORE + val + 4; } // +4 needed to return the highest card (ex:  going backwards, 2 trigger --> 6 is the highest card in the straight flush)  
            if(sum == 5) {
                return makeScore(STRAIGHTFLUSH_RANK, val + 4, 0, 0, 0, 0);
            }
        }

        // Special case:  Ace-low straight flush
        if(foundCards[suit][Two] && foundCards[suit][Three] && foundCards[suit][Four] && foundCards[suit][Five] && foundCards[suit][Ace]) {
            // return STRAIGHTFLUSH_SCORE + Five;
            return makeScore(STRAIGHTFLUSH_RANK, Five, 0, 0, 0, 0);
        }
    }
    
    // Wrapup:  Not a straight flush
    return 0;
}

int isAnteaterPair(PILES piles, int playerNum) {
    int playerNumBase = playerNum;

    if(piles.size() < 3 || playerNumBase < 0 || (unsigned)playerNumBase >= piles.size() - 2) {
        printf("ERROR: Can't determine if there is an Anteater pair if there are not enough piles or the wrong player number was provided.\n");
        return -1;
    }

    PILE hand = piles[playerNumBase];
    PILE comm = piles[piles.size() - 2];

    int anteaterCount = 0;

    for(Card x : hand) {
        if(x.getValue() == Anteater) {
            anteaterCount++;
        }
    }

    for(Card x : comm) {
        if(x.getValue() == Anteater) {
            anteaterCount++;
        }
    }

    if(anteaterCount >= 2) {
        return makeScore(ANTEATERPAIR_RANK, 0, 0, 0, 0, 0);
    }
    return 0;
}

int isFourKind(PILES piles, int playerNum) { // NOTE:  This type of hand DOES have a kicker
    int playerNumBase = playerNum; // in case there is a change/miscommunication about the purpose of player num (assuming player numbering starts at 0)
    // Error checking
    if(piles.size() < 3 || playerNumBase < 0 || (unsigned)playerNumBase >= piles.size() - 2) {
        printf("ERROR:  Can't determine if there is a four of a kind if there are not enough piles or the wrong player number was provided.\n");
        return -1;
    }

    // Vars
    PILE hand = piles[playerNumBase];
    PILE comm = piles[piles.size() - 2];
    
    // Checking for each value if hand + comm has all four
    for(int val = Ace; val >= Two; val--) { // have to go backwards to find the highest ranking hand
        int foundVal[4] = {0, 0, 0, 0};

        // Checking hole cards
        for(unsigned int i=0; i<hand.size(); i++) {
            if(hand[i].getSuit() < Hearts || hand[i].getSuit() > Spades) { // suit is out of range (possibly anteater?)
                continue;
            }
            if(hand[i].getValue() == val) {
                foundVal[hand[i].getSuit()] = 1;
            }

        }
        // Checking community cards
        for(unsigned int i = 0; i < comm.size(); i++) {
            if(comm[i].getSuit() < Hearts || comm[i].getSuit() > Spades){ // suit is out of range (possibly anteater?)
                continue;
            }
            if(comm[i].getValue() == val){
                foundVal[comm[i].getSuit()] = 1;
            }
        }

        int kicker = findKicker(piles, playerNumBase, val, -1, -1);

        if(foundVal[Hearts] + foundVal[Clubs] + foundVal[Diamonds] + foundVal[Spades] == 4) {
            // return(FOURKIND_SCORE + val * 20 + findKicker(piles, playerNumBase, val, -1, -1));
            return makeScore(FOURKIND_RANK, val, kicker, 0, 0, 0);
        }
    }

    // Wrapup:  Four of a kind not found--return 0
    return 0;
}

/* I don't think it works properly I just make three more diff func one pair, three of a kind, full house
//requires that pile comm is the entire 5 card community pile
int ofakind (PILES piles, int playerNum) {
    int playerNumBase = playerNum;
    if(piles.size() < 3 || playerNumBase < 0 || (unsigned)playerNumBase >= piles.size() - 2) {
        printf("ERROR:  Can't determine if there is a four of a kind if there are not enough piles or the wrong player number was provided.\n");
        return -1;
    }
    PILE hand = piles[playerNumBase];
    PILE comm = piles[piles.size() - 2];
    vector<int> mapped_tot;
    int temp;
    for (Card x : hand) {
        temp = val_translator(x.getValue());
        mapped_tot.push_back(temp);
    }
    for (Card x: comm) {
        temp = val_translator(x.getValue());
        mapped_tot.push_back(temp);
    }

    // total.insert(total.end(), comm.begin(), comm.end());
    // this line works same as two for loop above
    unordered_map<int,int> freq; //maps the values into frequency
    for(int y : mapped_tot) { // this loop is for count how many times each card value appears
        freq[y]++;
    }

    int max = 0;
    int second = 0;
    int valofmax = -1;
    int valofsecond = -1;

    // for(val x: freq) { //i forgot what the point of the second tracking vars was for, i dont think theyre necessary
    //    if(freq[x] > max || (freq[x] == max && x > valofmax)) { 
    //        second = max;
    //        max = freq[x];
    //        valofsecond = valofmax;
    //        valofmax = x;
    //    }
    // }
    for (auto const& entry : freq) { // second loop is to find the card value with the highest frequency
        int x = entry.first;
        int count = entry.second;

        if (count > max || (count == max && x > valofmax)) {
            second = max;
            max = count;
            valofsecond = valofmax;
            valofmax = x;
        }
    }
    //needs more logic for handling when the max count is all from the comm
    //1-2,3-4,7-8
    int score = 0;
    switch(max) {
        case 2: score += 1000; break;
        case 3: score += 3000; break;
        case 4: score += 7000; break;
        default: score += 0;   break;
    }
    if (max == 2 || max == 3 || max == 4) {
        score += valofmax;
    }

    return score;

}
*/

int isFullHouse(PILES piles, int playerNum) {
    int playerNumBase = playerNum;

    if(piles.size() < 3 || playerNumBase < 0 || (unsigned)playerNumBase >= piles.size() - 2) {
        printf("ERROR: Can't determine if there is a full house if there are not enough piles or the wrong player number was provided.\n");
        return -1;
    }

    PILE hand = piles[playerNumBase];
    PILE comm = piles[piles.size() - 2];

    int freq[15] = {0};

    // Count vals in hole cards
    for (Card x : hand) {
        int v = val_translator(x.getValue());
        if(v >= 2 && v <= 14) {
            freq[v]++;
        }
    }

    // Count vals in comm cards
    for (Card x : comm) {
        int v = val_translator(x.getValue());
        if(v >= 2 && v <= 14) {
            freq[v]++;
        }
    }

    int threeVal = -1;
    int pairVal = -1;
    // Find highest three of a kind
    for (int v = Ace; v >= Two; v--) {
        if(freq[v] >= 3) {
            threeVal = v;
            break;
        }
    }

    if(threeVal == -1) {
        return 0;
    }

    // Find highest pair that is not three of a kind
    for (int v = Ace; v >= Two; v--) {
        if(v != threeVal && freq[v] >= 2) {
            pairVal = v;
            break;
        }
    }

    if(pairVal == -1) {
        return 0;
    }

    // return 6000 + threeVal * 20 + pairVal;
    return makeScore(FULLHOUSE_RANK, threeVal, pairVal, 0, 0, 0);
}

int isThreeKind(PILES piles, int playerNum) {
    int playerNumBase = playerNum;

    if(piles.size() < 3 || playerNumBase < 0 || (unsigned)playerNumBase >= piles.size() - 2) {
        printf("ERROR: Can't determine if there is a three of a kind if there are not enough piles or the wrong player number was provided.\n");
        return -1;
    }

    PILE hand = piles[playerNumBase];
    PILE comm = piles[piles.size() - 2];

    int freq[15] = {0};

    // Count vals in hole cards
    for (Card x : hand) {
        int v = val_translator(x.getValue());
        if(v >= 2 && v <= 14) {
            freq[v]++;
        }
    }

    // Count vals in comm cards
    for (Card x : comm) {
        int v = val_translator(x.getValue());
        if(v >= 2 && v <= 14) {
            freq[v]++;
        }
    }

    int threeVal = -1;
    // Find highest three of a kind
    for (int v = Ace; v >= Two; v--) {
        if(freq[v] >= 3) {
            threeVal = v;
            break;
        }
    }

    if(threeVal == -1) {
        return 0;
    }

    // Find top two kickers
    int firstKicker = findKicker(piles, playerNumBase, threeVal, -1, -1);
    int secondKicker = findKicker(piles, playerNumBase, threeVal, firstKicker, -1);

    // return 3000 + threeVal * 400 + firstKicker * 20 + secondKicker;
    return makeScore(THREEKIND_RANK, threeVal, firstKicker, secondKicker, 0, 0);
}

int isTwoPair(PILES piles, int playerNum) {
    int playerNumBase = playerNum;

    if(piles.size() < 3 || playerNumBase < 0 || (unsigned)playerNumBase >= piles.size() - 2) {
        printf("ERROR: Can't determine if there is a two pair if there are not enough piles or the wrong player number was provided.\n");
        return -1;
    }

    PILE hand = piles[playerNumBase];
    PILE comm = piles[piles.size() - 2];
    
    int freq[15] = {0};

    // Count vals in hole cards
    for (Card x : hand) {
        int v = val_translator(x.getValue());
        if(v >= 2 && v <= 14) {
            freq[v]++;
        }
    }

    // Count vals in comm cards
    for (Card x : comm) {
        int v = val_translator(x.getValue());
        if(v >= 2 && v <= 14) {
            freq[v]++;
        }
    }

    int highPair = -1;
    int lowPair = -1;

    // Find two highest pairs
    for (int v = Ace; v >= Two; v--) {
        if(freq[v] >= 2) {
            if(highPair == -1) {
                highPair = v;
            }
            else {
                lowPair = v;
                break;
            }
        }
    }
    
    if(highPair == -1 || lowPair == -1) {
        return 0;
    }

    // Find kicker that is not a pair
    int kicker = findKicker(piles, playerNumBase, highPair, lowPair, -1);

    // return 2000 + highPair * 400 + lowPair * 20 + kicker;
    return makeScore(TWOPAIR_RANK, highPair, lowPair, kicker, 0, 0);
}

int isOnePair(PILES piles, int playerNum) {
    int playerNumBase = playerNum;

    if(piles.size() < 3 || playerNumBase < 0 || (unsigned)playerNumBase >= piles.size() - 2) {
        printf("ERROR: Can't determine if there is a one pair if there are not enough piles or the wrong player number was provided.\n");
        return -1;
    }

    PILE hand = piles[playerNumBase];
    PILE comm = piles[piles.size() - 2];

    int freq[15] = {0};

    // Count vals in hole cards
    for (Card x : hand) {
        int v = val_translator(x.getValue());
        if(v >= 2 && v <= 14) {
            freq[v]++;
        }
    }

    // Count vals in comm cards
    for (Card x : comm) {
        int v = val_translator(x.getValue());
        if(v >= 2 && v <= 14) {
            freq[v]++;
        }
    }

    int pairVal = -1;

    // Find highest pair
    for (int v = Ace; v >= Two; v--) {
        if(freq[v] >= 2) {
            pairVal = v;
            break;
        }
    }

    if(pairVal == -1) {
        return 0;
    }

    int firstKicker = findKicker(piles, playerNumBase, pairVal, -1, -1);
    int secondKicker = findKicker(piles, playerNumBase, pairVal, firstKicker, -1);
    int thirdKicker = findKicker(piles, playerNumBase, pairVal, firstKicker, secondKicker);

    // return 1000 + pairVal * 8000 + firstKicker * 400 + secondKicker * 20 + thirdKicker;
    return makeScore(ONEPAIR_RANK, pairVal, firstKicker, secondKicker, thirdKicker, 0);
} // Im not sure how the scoring works but if I don't give 8000 we can't consider thirdKicker 
  // lmk how the scoring works of fit these so that it works plz

int flush(PILES piles, int playerNum) {
    int playerNumBase = playerNum;

    if(piles.size() < 3 || playerNumBase < 0 || (unsigned)playerNumBase >= piles.size() - 2) {
        printf("ERROR: Can't determine if there is a flush if there are not enough piles or the wrong player number was provided.\n");
        return -1;
    }

    PILE hand = piles[playerNumBase];
    PILE comm = piles[piles.size() - 2];
    
    vector<int> suitVals[4];

    // Check hole cards
    for (Card x : hand) {
        int s = x.getSuit();
        int v = val_translator(x.getValue());

        if(s >= Hearts && s <= Spades && v >= 2 && v <= 14) {
            suitVals[s].push_back(v);
        }
    }

    // Check comm cards 
    for (Card x : comm) {
        int s = x.getSuit();
        int v = val_translator(x.getValue());

        if(s >= 1 && s <= 3 && v >= 2 && v <= 14) {
            suitVals[s].push_back(v);
        }
    }

    for (int s = Hearts; s <= Spades; s++) {
        if(suitVals[s].size() >= 5) {
            sort(suitVals[s].begin(), suitVals[s].end(), greater<int>());

            int high1 = suitVals[s][0];
            int high2 = suitVals[s][1];
            int high3 = suitVals[s][2];
            int high4 = suitVals[s][3];
            int high5 = suitVals[s][4];

            // return 5000 + high1 * 16000 + high2 * 8000 + high3 * 400 + high4 * 20 + high5;
            return makeScore(FLUSH_RANK, high1, high2, high3, high4, high5);
        }
    }

    return 0;
}

int straight(PILES piles, int playerNum){
    int playerNumBase = playerNum;

    PILE hand = piles[playerNumBase];
    PILE comm = piles[piles.size() - 2];

    vector<int>vals;

    int temp;
    int ace_check = 0, score = 0;

    for(Card x: comm){
        temp = val_translator(x.getValue());
        if(temp == 14) {ace_check = 1;}
        vals.push_back(temp);
    }
    for(Card y:hand){
        temp = val_translator(y.getValue());
        if(temp == 14){ace_check = 1;}
        vals.push_back(temp);
    }
    std::sort(vals.begin(), vals.end());
    if(ace_check == 1){ //just adds at most 1 ace to the vector
        vals.insert(vals.begin(), 1);
    }
    //just gets rid of repeating values
    /* for(int i = 0; i < vals.size() - 1; i++) {
        if(vals[i] == vals[i + 1]) {
            vals.erase(vals.begin() + i);
        }
    } */
   vals.erase(unique(vals.begin(), vals.end()), vals.end());
   // does same thing as the for loop above

    /* for(int i = 0; i < 3 - (7 + ace_check - vals.size()); i++) { //changing the initial cards
        if(vals[i] == vals[i + 1]) {continue;} //init condition of two same smallest values, skips to next index
        if(vals[i + 1] == vals[i] + 1) {
            for(int j = i + 1; j < i + 3; j++) {
                if(vals[j]!= vals[j + 1]) {
                    break;
                }
                else if(j == i + 2) { //if at the last index and if the prior condition is not true (consecutive)
                    score = 4000 + (vals[j + 1] / 15 * 1000) + vals[i] * 2; // scoring that acts sort of like base 14 (factors top 2 cards in hand)
                }
            }
        }
    } */
   for (int i = vals.size() - 1; i >= 4; i--) {
    if(vals[i] == vals[i - 1] + 1 &&
        vals[i - 1] == vals[i - 2] + 1 &&
        vals[i - 2] == vals[i - 3] + 1 &&
        vals[i - 3] == vals[i - 4] + 1) {
            // score = STRAIGHT_SCORE + vals[i]; // not sure we also have STRAIGHT_SCORE in scoreCalc.hpp
            return makeScore(STRAIGHT_RANK, vals[i], 0, 0, 0, 0);
            break;
        }
   } // check if its straight from the back(from the highest val)
    return score; 
} 

 // only compares highest two high card better be more but also I mentioned not sure bout how the scoring works may needs to be changed how the figure highest card set on the table
int highcard(PILES piles, int playerNum) {
    int playerNumBase = playerNum;

    PILE hand = piles[playerNumBase];
    PILE comm = piles[piles.size() - 2];

    vector<int> vals;

    for(Card x : hand) {
        int v = val_translator(x.getValue());
        if(v >= 2 && v <= 14) {
            vals.push_back(v);
        }
    }

    for(Card x : comm) {
        int v = val_translator(x.getValue());
        if(v >= 2 && v <= 14) {
            vals.push_back(v);
        }
    }

    if(vals.size() < 5) {
        return 0;
    }
    sort(vals.begin(), vals.end(), greater<int>());
    int high1 = vals[0];
    int high2 = vals[1];
    int high3 = vals[2];
    int high4 = vals[3];
    int high5 = vals[4];

    return high1 * 160000 + high2 * 8000 + high3 * 400 + high4 * 20 + high5;
}

int scoreHand(PILES piles, int playerNum) {
    int best = 0;
    int score = 0;

    score = isRoyalFlush(piles, playerNum);
    if(score > best) best = score;

    score = isStraightFlush(piles, playerNum);
    if(score > best) best = score;

    score = isAnteaterPair(piles, playerNum);
    if(score > best) best = score;

    score = isFourKind(piles, playerNum);
    if(score > best) best = score;

    score = isFullHouse(piles, playerNum);
    if(score > best) best = score;

    score = flush(piles, playerNum);
    if(score > best) best = score;

    score = straight(piles, playerNum);
    if(score > best) best = score;

    score = isThreeKind(piles, playerNum);
    if(score > best) best = score;

    score = isTwoPair(piles, playerNum);
    if(score > best) best = score;

    score = isOnePair(piles, playerNum);
    if(score > best) best = score;

    score = highcard(piles, playerNum);
    if(score > best) best = score;

    return best;
}

int val_translator(int x) {
    switch(x){
        case Two: return 2;
        case Three: return 3;
        case Four: return 4;
        case Five: return 5;
        case Six: return 6;
        case Seven: return 7;
        case Eight: return 8;
        case Nine: return 9;
        case Ten: return 10;
        case Jack: return 11;
        case Queen: return 12;
        case King: return 13;
        case Ace: return 14;
        //something needs to be done about the anteater

        default: return 0;
    }
}

int makeScore(int hand, int max, int second, int third, int fourth, int fifth) {
    return hand   * 0x100000 +
           max    * 0x010000 +
           second * 0x001000 +
           third  * 0x000100 +
           fourth * 0x000010 +
           fifth;
}