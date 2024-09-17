#ifndef NEGAMAX_H
#define NEGAMAX_H

#include "evaluation.h"
#include "transposition.h"
#include <map>
using namespace chess;

std::string position(Color player, Square square_from, Square square_to);

class Negamax
{
public:
    int depth;
    int curr_depth;
    Evaluation* model;
    std::unique_ptr<TranspositionTable> table = std::make_unique<TranspositionTable>();
    std::map<std::string, int> history = std::map<std::string, int>();
    

public:
    Negamax() : depth(1), model(new Evaluation()) {this->curr_depth = 1;};
    Negamax(int depth, Evaluation* model) : depth(depth), model(model) {this->curr_depth = 1;};
    
    void moveOrdering(Board &board, Movelist &moves, int local_depth);
    Move iterative_deepening(Board &board);

    Move best(Board &board, int depth);
    int quiescience(Board &board, int alpha, int beta);

private:
    int best_priv(Board &board, int depth, int alpha, int beta);
    
};

#endif