#ifndef NEGAMAX_H
#define NEGAMAX_H

#include "evaluation.h"
#include "transposition.h"
#include <map>
#include <chrono>
#include <atomic>
using namespace chess;

std::string position(Color player, Square square_from, Square square_to);

class Negamax
{
public:
    int depth;
    int num_threads;
    std::chrono::time_point<std::chrono::high_resolution_clock> time_start_search;
    int time_move_ms;
    bool interrupt = false;
    bool is_time_finished = false;
    Evaluation* model;
    std::shared_ptr<TranspositionTable> table = std::make_shared<TranspositionTable>();
    std::shared_ptr<std::map<std::string, int>> history = std::make_shared<std::map<std::string, int>>();
    std::shared_ptr<std::map<int, std::pair<Move,Move>>> killer_moves = std::make_shared<std::map<int, std::pair<Move,Move>>>();
    

public:
    Negamax() : depth(1), model(new Evaluation()) {time_move_ms=10000;num_threads=4;};
    Negamax(int depth, Evaluation* model) : depth(depth), model(model) {time_move_ms=10000;num_threads=4;};

    void moveOrdering(Board &board, Movelist &moves, int local_depth);
    void setScoreAttackingMove(chess::Board &board, chess::Move &move, chess::Piece &pieceTo);
    Move iterative_deepening(Board &board);
    bool isBestMoveMate(chess::Board &board, const chess::Move &best_move_until_now);
    bool time_end();
    Score best(Board& board, int depth);
    void bestMoveThread(Board board, int local_depth, int j_thread);
    int quiescence(Board &board, int alpha, int beta, int quiescence_depth, int ply, int& numNodes);
    bool isThereAMajorPiece(Board &board);
    int differenceMaterialWhitePerspective(Board &board);
    void resetTT();
    std::pair<GameResultReason, GameResult> isDraw(Board& board);
    std::pair<GameResultReason, GameResult> isCheckmate(Board &board);

private:
    int best_priv(Board &board, int depth, int alpha, int beta, int& numNodes, int ply, bool can_null);
    
    
};

#endif