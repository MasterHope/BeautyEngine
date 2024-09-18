#include "engine.h"
#include "evaluation.h"
#include "negamax.h"
#include "chess-library-master/include/chess.hpp"
#include <future>
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
      curr_board(std::make_unique<Board>()), // Use unique_ptr for Board
      model(std::make_unique<PestoEvaluation>()), // Use unique_ptr for Simple or Pesto...
      negamax(std::make_unique<Negamax>(6, model.get())) // Use unique_ptr for Negamax
{
}
void Engine::go(){
    this->best_move_last_iter = this->negamax->iterative_deepening(*this->curr_board);
    this->curr_board->makeMove(best_move_last_iter);
}

void Engine::position(std::string& fen){
    this->curr_board->setFen(fen);
}

void Engine::stop(std::future<void>* bestThread){
    this->negamax.get()->stop = true;
    bestThread->wait();
}

