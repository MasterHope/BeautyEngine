#include "transposition.h"
#include "chess-library-master/include/chess.hpp"
#include<iostream>
using namespace chess;
//logging
//#define LOGGING_TT
//cleaning if it is required...
//#define CLEANING
void TranspositionTable::store(Board &board, TTEntry ttEntry){
    uint64_t index = this->hash(board);
    #ifdef CLEANING
        if (this->mustClean()){
            #ifdef LOGGING_TT
            std::clog<<"Transposition table cleaning process"<<std::endl;
            std::clog<<"Number of elements before the process:"<<num_elements<<std::endl;        
            #endif
            
            this->cleanUp(board.halfMoveClock());
            #ifdef LOGGING_TT
            std::clog<<"Number of elements after the process:"<<num_elements<<std::endl;
            #endif
        }
    #endif
    if (this->hasCollisionAt(index)){
        this->replace(index, ttEntry);
    } else {
        #pragma omp critical
        {
            this->tt[index] = ttEntry;
            this->num_elements++;
        }
    }
};

bool TranspositionTable::mustClean(){
    //partial clean up when the table is occupied more than 90 percent.
    return num_elements >= (int(TTSIZE*0.90));
}

void TranspositionTable::cleanUp(uint32_t clock){
    int init_element = num_elements;
    for (int i = 0; i < TTSIZE; i++){
        TTEntry ttEntry = tt[i];
        if (ttEntry.flag != EMPTY){
            if (ttEntry.age < clock){
                #pragma omp critical
                {
                    tt[i] = TTEntry();
                    num_elements--;
                }
                if (num_elements <= init_element * 0.40){
                    return;
                }
            }
        }
    }
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
        #pragma omp atomic
        this->tt[index] = ttEntry;
    }
};
