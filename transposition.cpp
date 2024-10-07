#include "transposition.h"
#include "chess-library-master/include/chess.hpp"
#include<iostream>
using namespace chess;

std::mutex tt_m;

//logging
//#define LOGGING_TT
void TranspositionTable::store(Board &board, TTEntry ttEntry){
    uint64_t index = this->hash(board);
    if (this->hasCollisionAt(index)){
        this->replace(index, ttEntry);
    } else {
            std::lock_guard lk(tt_m);
            {
            this->tt[index] = ttEntry;
            this->num_elements++;
            }
    }
};

TTEntry TranspositionTable::lookup(Board &board){
    int index = this->hash(board);
    TTEntry ttEntry = this->tt[index];
    return (board.hash()==ttEntry.hash) ? ttEntry : TTEntry();
}
void TranspositionTable::replace(int index, TTEntry ttEntry){
    //substitute if the new entry is something an higher depth...
    TTEntry currEntry = this->tt[index];
    if (currEntry.depth < ttEntry.depth && currEntry.age < ttEntry.age){
        {
        std::lock_guard lk(tt_m);
        this->tt[index] = ttEntry;
        }
    }
};

void TranspositionTable::updateAge(int index, uint32_t clock){
    {
        std::lock_guard lk(tt_m);
        tt[index].age = clock;
    }
}