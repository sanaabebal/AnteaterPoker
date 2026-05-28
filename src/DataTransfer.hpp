/* DataTransfer.hpp */


#ifndef DataTransfer_H
#define DataTransfer_H

#include <vector>
#include "cards.hpp"
#include "data.hpp"

BUF createBuffer(GAMESTATE gameState);
BUF createBuffer(LOGININFO loginInfo);

GAMESTATE parsingGameArguments(BUF &buf);
LOGININFO parsingLoginArguments(BUF &buf);


#endif