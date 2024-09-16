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
    std::unique_ptr<Evaluation> model;
    std::unique_ptr<TranspositionTable> table = std::make_unique<TranspositionTable>();
    std::map<std::string, int> history = std::map<std::string, int>();
    

public:
    Negamax(int depth, std::unique_ptr<Evaluation> model) : depth(depth), model(std::move(model)) {};
    
    Movelist moveOrdering(Board &board, Movelist &moves);
    Move iterative_deepening(Board &board);
    int quiescence(Board &board, int alpha, int beta);

private:
    int best_priv(Board &board, int depth, int alpha, int beta);
    std::pair<Move, int> best(Board &board, int depth);
};

#endif