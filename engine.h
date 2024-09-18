#ifndef ENGINE_H
#define ENGINE_H
#include "chess-library-master/include/chess.hpp"
#include "negamax.h"
#include <string>
#include <future>

using namespace chess;

class Engine{
    public:
        std::string info;
        std::vector<std::string> options;
        Move best_move_last_iter;
        std::unique_ptr<Board> curr_board;
        std::unique_ptr<Evaluation> model;
        std::unique_ptr<Negamax> negamax;

    public:
        Engine();
        void position(std::string& fen);
        void go();
        void stop(std::future<void>* bestThread);
        void quit() {};
        void make_move(Move move);

};
#endif