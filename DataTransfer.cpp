/* PokerDataTransfer.cpp */
#include <vector>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "data.hpp"
#include "cards.hpp"

//typedef std::vector<unsigned char> BUF;

/* Packing Helper Functions*/

template<typename T>
void addToBuf(BUF &buf, T x) {
    buf.resize(buf.size() + sizeof(x));
    unsigned char *bufEnd = buf.data() + buf.size();
    unsigned char *newVal = bufEnd - sizeof(x);
    memcpy(newVal, &x, sizeof(x));
}

template<size_t n>
void addCharArrToBuf(BUF &buf, char (&arr)[n]){
    unsigned char size = n;
    addToBuf(buf, size);
    buf.insert(buf.end(), arr, arr+n);

    if(size != 41){
        printf("WARNING:  Character string should be 41 characters, but isn't--errors may occur.\n");
    }

}

template<typename T>
void addVecToBuf(BUF &buf, std::vector<T> &vec) { // don't have to dereference--& means essentailly "pass by reference, but don't have to treat it like a pointer"
    // Data size header
    addToBuf(buf, vec.size());
    // Adding vector
    for(unsigned int i=0; i<vec.size(); i++){
        addToBuf(buf, vec[i]);
    }
}

template<typename T>
void addVecVecToBuf(BUF &buf, std::vector<std::vector<T>> &vec) {
    addToBuf(buf, vec.size()); // size of VecVec
    for (unsigned int i=0; i<vec.size(); i++) {
        addVecToBuf(buf, vec[i]);
    }
}

/* PACKING */
BUF createBuffer(GAMESTATE gameState){
    BUF answer;
    //addToBuf(answer, gameState.packageType);
    // First entry:  packageType (int:  0 or 1)
    addToBuf(answer, gameState.packageType); // takes up one vector element
    // Second entry:  number of players (int:  most likely between 3 and 10)
    addToBuf(answer, gameState.numPlayers); // takes up one vector element
    // Third entry:  round number (int:  between 0 and 4)
    addToBuf(answer, gameState.round); // takes up one vector element
    // Fourth entry:  dealer player (int:  most likely between 0 and 9)
    addToBuf(answer, gameState.dealerPlayer);
    // Fifth entry:  pot (int)
    addToBuf(answer, gameState.pot);
    // Sixth entry:  call amount (int)
    addToBuf(answer, gameState.callAmount);
    // Seventh entry:  player turn (int)
    addToBuf(answer, gameState.playerTurn);

    // Eigth entry:  PILES allCards
    addVecVecToBuf(answer, gameState.allCards);

    // Ninth entry:  PLAYERS players
    addVecToBuf(answer, gameState.players);

    // Wrapup
    return answer;


    #ifdef IGNORE
    //  entry:  PILES allCards (size + data)
        // PILES size (int)
        answer.push_back(gameState.allCards.size()); // unsigned int:  takes up one vector element
        // each PILE needs to have its own size + data
        for(unsigned int i=0; i<gameState.allCards.size(); i++){
            answer.push_back(gameState.allCards[i].size()); // unsigned int:  takes up one vector element
            // answer.insert()
        }
    #endif


}

BUF createBuffer(LOGININFO loginInfo){
    BUF answer;
    addToBuf(answer, loginInfo.packageType);
    addToBuf(answer, loginInfo.isHost);
    addCharArrToBuf(answer, loginInfo.playerName);
    addToBuf(answer, loginInfo.playerNum);
    addCharArrToBuf(answer, loginInfo.password);
    addToBuf(answer, loginInfo.playerType);
    addToBuf(answer, loginInfo.numPlayers);
    addVecToBuf(answer, loginInfo.playersFound);

    // Wrapup
    return answer;
}



/* Unpacking Helper Functions */

template<typename T> // tied to function
void processBuf(BUF &buf, int &index, T &answer){ // T can't be ret value b/c it could have different values (resulting in same funct inputs and name, different output--not allowed overloading type)
    assert(index >= 0 && index < buf.size());

    memcpy(&answer, &buf[index], sizeof(T)); // addr_dest, addr_src, size of value
    index += sizeof(T);
}

template<size_t n> // tied to function
void processCharArrBuf(BUF &buf, int &index, char (&answer)[n]){
        assert(index >= 0 && index < buf.size());
        int size = buf[index];
        assert(index + size + 1 <= buf.size()); // 1 byte for size header
        assert(size <= n);
        index++;

        if(size != 41){
            printf("WARNING:  Character string should be 41 characters, but isn't--errors may occur.\n");
        }

        memcpy(answer, buf.data() + index, size); // dest, src, size

        // Wrapup
        answer[n-1] = '\0';
        index += size;
}

template<typename T>
void processVecBuf(BUF &buf, int &index, std::vector<T> &answer){ // see overloading notes regarding processBuf()
    // Pull the header (size)
    size_t vecSize = 0; // vector.size() returns size_t -- need to probably check for this in other spots in the code...
    processBuf(buf, index, vecSize);
    assert(vecSize < 100); // probably should be less than this, but not checking this right now...should come back to this 
    
    answer.resize(vecSize);
    for(size_t i=0; i<vecSize; i++){
        processBuf(buf, index, answer[i]); // index updated accordingly in this function--don't have to worry about it here
    }
}

template<typename T>
void processVecVecBuf(BUF &buf, int &index, std::vector<std::vector<T>> &answer){
    // Pull the header (size)
    size_t vecVecSize = 0;
    processBuf(buf, index, vecVecSize); // setting vecVecSize
    assert(vecVecSize < 100); // probably should be less than this, but not checking right now

    // Writing to answer
    answer.resize(vecVecSize);
    for(size_t i=0; i<vecVecSize; i++){
        processVecBuf(buf, index, answer[i]);
    }
}


/* UNPACKING */

GAMESTATE parsingGameArguments(BUF &buf){
    GAMESTATE answer;
    int index = 0;
    // MUST BE IN SAME ORDER AS SERIALIZATION
    processBuf(buf, index, answer.packageType);
    processBuf(buf, index, answer.numPlayers);
    processBuf(buf, index, answer.round);
    processBuf(buf, index, answer.dealerPlayer);
    processBuf(buf, index, answer.pot);
    processBuf(buf, index, answer.callAmount);
    processBuf(buf, index, answer.playerTurn);


    // Eigth entry:  PILES allCards
    processVecVecBuf(buf, index, answer.allCards);

    // Ninth entry:  PLAYERS players
    processVecBuf(buf, index, answer.players);

    // Wrapup
    return answer;
}

LOGININFO parsingLoginArguments(BUF &buf){ // pass by reference (doesn't impact function calls)
    LOGININFO answer;
    int index = 0;

    // MUST BE IN SAME ORDER AS SERIALIZATION
    processBuf(buf, index, answer.packageType);
    processBuf(buf, index, answer.isHost);
    processCharArrBuf(buf, index, answer.playerName);
    processBuf(buf, index, answer.playerNum);
    processCharArrBuf(buf, index, answer.password);
    processBuf(buf, index, answer.playerType);
    processBuf(buf, index, answer.numPlayers);

    processVecBuf(buf, index, answer.playersFound);


    // Wrapup
    return answer;
}