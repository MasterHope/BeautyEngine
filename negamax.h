#ifndef NEGAMAX_H
#define NEGAMAX_H

#include "evaluation.h"
#include "transposition.h"
#include <map>
#include <atomic>
using namespace chess;

std::string position(Color player, Square square_from, Square square_to);

class Negamax
{
public:
    int depth;
    int curr_depth;
    int ply;
    int numNodes;
    time_t time_start_search;
    int time_move_seconds;
    std::atomic<bool> stop{false};
    Evaluation* model;
    std::unique_ptr<TranspositionTable> table = std::make_unique<TranspositionTable>();
    std::map<std::string, int> history = std::map<std::string, int>();
    

public:
    Negamax() : depth(1), model(new Evaluation()) {this->curr_depth = 1;this->ply=0;this->numNodes=0;time_move_seconds=60;};
    Negamax(int depth, Evaluation* model) : depth(depth), model(model) {this->curr_depth = 1;this->ply=0;this->numNodes=0;time_move_seconds=60;};

    void moveOrdering(Board &board, Movelist &moves, int local_depth);
    void setScoreAttackingMove(chess::Board &board, chess::Move &move, chess::Piece &pieceTo);
    Move iterative_deepening(Board &board);
    bool time_end();
    Move best(Board &board, int depth);
    int quiescence(Board &board, int alpha, int beta, int quiescence_depth);

private:
    int best_priv(Board &board, int depth, int alpha, int beta);
    
};

#endif