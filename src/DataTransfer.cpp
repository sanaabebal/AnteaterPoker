#include "DataTransfer.hpp"
#include <cstring>
#include <cassert>
#include <cstdio>

template<typename T> 
void processBuf(BUF &buf, size_t &index, T &answer){ 
    assert(index < buf.size());
    memcpy(&answer, &buf[index], sizeof(T));
    index += sizeof(T);
}

template<size_t n> 
void processCharArrBuf(BUF &buf, size_t &index, char (&answer)[n]){
    assert(index < buf.size());
    int size = buf[index];
    assert(index + size + 1 <= buf.size());
    assert(size <= (int)n);
    index++;

    if(size != 41){
        printf("WARNING: Character string should be 41 characters, but isn't.\n");
    }

    memcpy(answer, buf.data() + index, size);
    answer[n-1] = '\0';
    index += size;
}

template<typename T>
void processVecBuf(BUF &buf, size_t &index, std::vector<T> &answer){ 
    size_t vecSize = 0; 
    processBuf(buf, index, vecSize);
    assert(vecSize < 100); 
    
    answer.resize(vecSize);
    for(size_t i = 0; i < vecSize; i++){
        processBuf(buf, index, answer[i]); 
    }
}

template<typename T>
void processVecVecBuf(BUF &buf, size_t &index, std::vector<std::vector<T>> &answer){
    size_t vecVecSize = 0;
    processBuf(buf, index, vecVecSize); 
    assert(vecVecSize < 100); 

    answer.resize(vecVecSize);
    for(size_t i = 0; i < vecVecSize; i++){
        processVecBuf(buf, index, answer[i]);
    }
}


template<>
void processBuf<Card>(BUF &buf, size_t &index, Card &answer) {
    assert(index + sizeof(Card) <= buf.size());
    answer = *reinterpret_cast<const Card*>(&buf[index]);
    index += sizeof(Card);
}

template<>
void processBuf<Player>(BUF &buf, size_t &index, Player &answer) {
    assert(index + sizeof(Player) <= buf.size());
    answer = *reinterpret_cast<const Player*>(&buf[index]);
    index += sizeof(Player);
}


GAMESTATE parsingGameArguments(BUF &buf){
    GAMESTATE answer;
    size_t index = 0; 
    processBuf(buf, index, answer.packageType);
    processBuf(buf, index, answer.numPlayers);
    processBuf(buf, index, answer.round);
    processBuf(buf, index, answer.dealerPlayer);
    processBuf(buf, index, answer.pot);
    processBuf(buf, index, answer.callAmount);
    processBuf(buf, index, answer.playerTurn);
    processBuf(buf, index, answer.greatest); 

    processVecVecBuf(buf, index, answer.allCards);
    processVecBuf(buf, index, answer.players);
    return answer;
}

LOGININFO parsingLoginArguments(BUF &buf){ 
    LOGININFO answer;
    size_t index = 0; 

    processBuf(buf, index, answer.packageType);
    processBuf(buf, index, answer.isHost);
    processCharArrBuf(buf, index, answer.playerName);
    processBuf(buf, index, answer.playerNum);
    processCharArrBuf(buf, index, answer.password);
    processBuf(buf, index, answer.playerType);
    processBuf(buf, index, answer.numPlayers);

    processVecBuf(buf, index, answer.playersFound);
    return answer;
}

template void processBuf<int>(BUF&, size_t&, int&);
template void processBuf<size_t>(BUF&, size_t&, size_t&);

template void processCharArrBuf<41>(BUF&, size_t&, char (&)[41]);

template void processVecBuf<int>(BUF&, size_t&, std::vector<int>&);
template void processVecBuf<Card>(BUF&, size_t&, std::vector<Card>&);
template void processVecBuf<Player>(BUF&, size_t&, std::vector<Player>&);

template void processVecVecBuf<Card>(BUF&, size_t&, std::vector<std::vector<Card>>&);