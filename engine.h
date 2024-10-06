#ifndef ENGINE_H
#define ENGINE_H
#include "chess-library-master/include/chess.hpp"
#include "negamax.h"
#include <string>
#include <atomic>

#define STARTINGFEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

using namespace chess;

class Engine{
    public:
        std::string info;
        std::vector<std::string> options;
        Move best_move_last_iter;
        std::shared_ptr<Board> curr_board;
        std::shared_ptr<Evaluation> model;
        std::shared_ptr<Negamax> negamax;
        std::atomic<bool> isSearching{false};

    public:
        Engine();
        void position(std::string& fen);
        void go();
        void quit() {};
        void make_move(Move move);
        void setTime(int timems){this->negamax.get()->time_move_ms = timems;};
        void reset();

};
#endif