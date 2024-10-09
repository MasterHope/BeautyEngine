#ifndef NEGAMAX_H
#define NEGAMAX_H

#include "evaluation.h"
#include "transposition.h"
#include <map>
#include <chrono>
#include <atomic>
#include <thread>
#include <array>
using namespace chess;

std::string position(Color player, Square square_from, Square square_to);

class Negamax
{
public:
    static constexpr int depth = 128;
    int num_threads;
    std::chrono::time_point<std::chrono::high_resolution_clock> time_start_search;
    int time_move_ms;
    std::atomic_bool interrupt{false};
    std::atomic_bool is_time_finished{false};
    Evaluation* model;
    std::shared_ptr<TranspositionTable> table = std::make_shared<TranspositionTable>();
    std::shared_ptr<std::array<std::array<std::array<int16_t ,64>, 64>, 2>> history = std::make_shared<std::array<std::array<std::array<int16_t ,64>, 64>, 2>>();
    std::shared_ptr<std::array<std::pair<Move,Move>, depth>> killer_moves;
    

public:
    Negamax() : model(new Evaluation()) {time_move_ms=10000;num_threads=std::thread::hardware_concurrency(); init_killer(true); init_history(true);};
    Negamax(Evaluation* model) : model(model) {time_move_ms=10000;num_threads=std::thread::hardware_concurrency(); init_killer(true);init_history(true);};

    void moveOrdering(Board &board, Movelist &moves, int local_depth, int ply);
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
    void init_killer(bool reset);
    void init_history(bool reset);
    Move getSmallestAttackerMove(Board& board, Square square, Color color);
    int see(Board& board, Square square, Color color);
private:
    int best_priv(Board &board, int depth, int alpha, int beta, int &numNodes, int ply, bool can_null);
    void updateKillers(int local_depth, const chess::Move &move);
    void updateHistory(chess::Board &board, chess::Move &move, int ply);
};

#endif