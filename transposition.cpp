#include "transposition.h"
#include "chess-library-master/include/chess.hpp"
using namespace chess;



void TranspositionTable::store(Board &board, TTEntry ttEntry){
    uint64_t index = this->hash(board);
    if (this->mustClean()){
        this->cleanUp(board.halfMoveClock());
    }
    if (this->hasCollisionAt(index)){
        this->replace(index, ttEntry);
    } else {
        this->tt[index] = ttEntry;
        this->num_elements++;
    }
};

bool TranspositionTable::mustClean(){
    return false;
}

void TranspositionTable::cleanUp(uint32_t clock){
    return;
}

TTEntry TranspositionTable::lookup(Board &board){
    int index = this->hash(board);
    TTEntry ttEntry = this->tt[index];
    return (board.hash()==ttEntry.hash) ? ttEntry : TTEntry();
}
void TranspositionTable::replace(int index, TTEntry ttEntry){
    //substitute if the new entry is something an higher depth...
    TTEntry currEntry = this->tt[index];
    if (currEntry.depth < ttEntry.depth && currEntry.age < ttEntry.age){
        this->tt[index] = ttEntry;
    }
};
