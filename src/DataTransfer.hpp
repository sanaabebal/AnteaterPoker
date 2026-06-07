/* DataTransfer.hpp */

#ifndef DataTransfer_H
#define DataTransfer_H

#include <vector>
#include <cstring>
#include "cards.hpp"
#include "data.hpp"

typedef std::vector<unsigned char> BUF;

template<typename T> 
void processBuf(BUF &buf, size_t &index, T &answer);

template<size_t n> 
void processCharArrBuf(BUF &buf, size_t &index, char (&answer)[n]);

template<typename T>
void processVecBuf(BUF &buf, size_t &index, std::vector<T> &answer);

template<typename T>
void processVecVecBuf(BUF &buf, size_t &index, std::vector<std::vector<T>> &answer);

GAMESTATE parsingGameArguments(BUF &buf);
LOGININFO parsingLoginArguments(BUF &buf);

template <typename T>
inline BUF createBuffer(const T& data) {
    BUF buffer(sizeof(T));
    std::memcpy(buffer.data(), &data, sizeof(T));
    return buffer;
}

inline BUF createBuffer(const LOGININFO& info) {
    BUF buf;
    
    auto append = [&](const auto& value) {
        const unsigned char* p = reinterpret_cast<const unsigned char*>(&value);
        buf.insert(buf.end(), p, p + sizeof(value));
    };

    auto appendStr = [&](const char* charArr, size_t maxSize) {
        size_t actualLen = std::strlen(charArr);
        
        if (actualLen >= maxSize) actualLen = maxSize - 1;
        
        buf.push_back(static_cast<unsigned char>(actualLen));
        
        buf.insert(buf.end(), charArr, charArr + actualLen);
    };

    append(info.packageType);
    append(info.isHost);
    
    appendStr(info.playerName, sizeof(info.playerName));
    
    append(info.playerNum);
    
    appendStr(info.password, sizeof(info.password));
    
    append(info.playerType);
    append(info.numPlayers);
    
    size_t vecSize = info.playersFound.size();
    append(vecSize);
    for (size_t i = 0; i < vecSize; ++i) {
        append(info.playersFound[i]);
    }

    return buf;
}

inline BUF createBuffer(const GAMESTATE& state) {
    BUF buf;
    
    auto append = [&](const auto& value) {
        const unsigned char* p = reinterpret_cast<const unsigned char*>(&value);
        buf.insert(buf.end(), p, p + sizeof(value));
    };

    append(state.packageType);
    append(state.numPlayers);
    append(state.round);
    append(state.dealerPlayer);
    append(state.pot);
    append(state.callAmount);
    append(state.playerTurn);
    append(state.greatest);

    size_t numPiles = state.allCards.size();
    append(numPiles);
    for (size_t i = 0; i < numPiles; ++i) {
        size_t pileSize = state.allCards[i].size();
        append(pileSize);
        for (size_t j = 0; j < pileSize; ++j) {
            append(state.allCards[i][j]);
        }
    }

    // Write vector<Player>
    size_t numPlayers = state.players.size();
    append(numPlayers);
    for (size_t i = 0; i < numPlayers; ++i) {
        append(state.players[i]);
    }

    return buf;
}

#endif