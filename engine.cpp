#include "engine.h"
#include "evaluation.h"
#include "negamax.h"
#include "chess-library-master/include/chess.hpp"
using namespace chess;

Engine::Engine()
    : info("Welcome to my personal Chess Engine!"),
      options{"uci | set uci for moves \n",
               "isready | used to synchronized engine with GUI \n",
               "setoption | used to set option of the engine \n",
               "ucinewgame | start a new game\n",
               "position | set new position given a fen of a board\n",
               "go | start calculating for the given position\n"},
      best_move_last_iter(Move()), // Default initialization
      curr_board(std::make_shared<Board>()), 
      model(std::make_shared<PestoEvaluation>()), 
      negamax(std::make_shared<Negamax>(model.get()))
{
}
void Engine::go(){
    this->best_move_last_iter = this->negamax->iterative_deepening(*this->curr_board);
    this->curr_board->makeMove(best_move_last_iter);
    std::cout << "bestmove " << uci::moveToUci(this->best_move_last_iter) << std::endl;
}

void Engine::position(std::string& fen){
    this->curr_board->setFen(fen);
}

void Engine::reset(){
    negamax->interrupt.store(false);
    negamax->is_time_finished.store(false);
    negamax->resetTT();
    negamax->init_history(false);
    best_move_last_iter = Move();
}



