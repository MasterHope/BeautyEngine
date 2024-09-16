#ifndef TRANSPOSITION_H
#define TRANSPOSITION_H
#include "chess-library-master/include/chess.hpp"
#include <memory>
#include <iostream>
using namespace chess;

constexpr std::size_t TTSIZE = static_cast<std::size_t>(1) << 16;

#define EMPTY -1
#define EXACT 0
#define LOWERBOUND 1
#define UPPERBOUND 2


struct TTEntry{
    uint64_t hash = -1;
    int value = -1;
    int flag = EMPTY;
    int depth = -1;
    bool isvalid = false;
    Move bestMove = Move();
};

class TranspositionTable{
    private:
        TTEntry tt[TTSIZE] = {TTEntry()};
        int num_elements = 0;

    public:
        TTEntry lookup(Board &board);
        void store(Board &board, TTEntry ttEntry);
        int hash(Board &board) {return board.hash() % TTSIZE;};
    
    private:
        void replace(int index, TTEntry ttEntry);
        bool hasCollisionAt(int index){return this->tt[index].isvalid;};

};

#endif