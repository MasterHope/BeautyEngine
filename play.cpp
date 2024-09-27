#include "evaluation.h"
#include "negamax.h"
#include "engine.h"
#include <iostream>
#include <string>
#include <future>

#include <typeinfo>
#include "play.h"
using namespace chess;

//some positions to test...
//for black...2r1kb2/pp2p2r/2p2n1p/1q1p1pp1/P2P2b1/N5PP/1PPBPPB1/3RKR2 b - - 2 17
//for white...rnbqkbnr/2pp1ppp/1p6/p3p3/2B1P3/5Q2/PPPP1PPP/RNB1K1NR w KQkq - 0 4
//3r2k1/pp4p1/2p3pn/2Pp2q1/3PPr2/P4B2/1B3P2/2RQ1R1K w - - 1 29 -> mate risk (for some reason i left with mate in one to the opponent, but I don't know why...)
//mate in 6 moves... 8/8/5R2/8/3K1k2/8/3N4/8 b - - 9 5
//mate in 2 moves... 8/8/5R2/8/4N2k/5K2/8/8 b - - 17 9
//mate in 3 moves... 8/8/5R2/8/4N2k/8/5K2/8 w - - 16 9
//mate in 4 moves... 6k1/pp4p1/2p5/2P3K1/3P4/P1R2r2/1B2q3/8 b - - 0 39
//mate in 5 moves 8/8/8/7R/8/2p2Pk1/2P1K3/8 w - - 1 7
//mate in 1 8/2B5/8/8/2k4p/K2b4/8/1q6 w - - 8 152
//black is losing... 8/8/2n5/4k3/1R6/2K5/3N4/8 w - - 0 1
//free exchange rnbqkbnr/pppp1ppp/8/4p3/3P4/8/PPP1PPPP/RNBQKBNR w KQkq - 0 2
//a lot of changes... 4r3/ppqp1kbp/8/4P3/8/2BN1n1P/PPK3P1/8 b - - 0 1
//no main pieces present...8/pp1p1k1p/8/3P2P1/6P1/8/P1K5/8 b - - 0 8
    

void isThereAMajorPiece(){
    Engine engine = Engine();
    engine.curr_board.get()->setFen("8/pp1p1k1p/8/3P2P1/6P1/8/P1K5/8 b - - 0 8");
    std::cout<<engine.negamax.get()->isThereAMajorPiece(*engine.curr_board)<<std::endl;
}

void testEngine(){
    Engine engine = Engine();
    engine.curr_board.get()->setFen("8/8/5R2/8/4N2k/5K2/8/8 b - - 17 9");
    while (engine.curr_board.get()->isGameOver().first == GameResultReason::NONE){
        engine.go();
        std::cout<<engine.best_move_last_iter<<std::endl;
    }
}
// command... g++ play.cpp evaluation.cpp negamax.cpp -o play
// test = "2r5/5k2/8/1K6/8/3Q4/8/8 w - - 0 1"

void play(chess::Board &board, Negamax &negamax)
{
    while (board.isGameOver().first == GameResultReason::NONE)
    {
        Move max_move = negamax.iterative_deepening(board);
        std::cout << "best:" << max_move << std::endl;
        board.makeMove(max_move);
    }
}



void uci_loop()
{
    std::string command, token;
    Engine engine = Engine();
     while (std::getline(std::cin, command))
    {
        std::istringstream is(command);
        token.clear();
        is >> std::skipws >> token;
        if (token == "uci")
        {
            std::cout << engine.info << std::endl;
            std::cout << "uciok" << std::endl;
        }
        else if (token == "isready")
        {
            std::cout << "readyok" << std::endl;
        }
        else if (token == "position")
        {
            std::string option;
            is >> std::skipws >> option;
            if (option == "startpos")
            {
                engine.curr_board = std::make_unique<Board>();
                std::string moves;
                is >> std::skipws >> moves;
                if (!moves.empty())
                {
                    std::string move;
                    while (is >> std::skipws >> move)
                    {
                        engine.curr_board->makeMove(uci::uciToMove(*engine.curr_board.get(),move)); 
                    }

                }
            }
            else if (option == "fen")
            {
                std::string line_fen; 
                getline(is, line_fen);
                engine.curr_board.get()->setFen(line_fen);
            }
        }
        else if (token == "newgame")
        {
            engine = Engine();
        }
        else if (token == "go")
        {
            is >> std::skipws >> token;
            if (token=="movetime"){
                is >> std::skipws >> token;
                engine.setTime(atoi(token.c_str()));
            }
            if (engine.curr_board.get()->isGameOver().first == GameResultReason::NONE)
            {
                //thread doing search
                std::future<void> bestThread = std::async(std::launch::async, &Engine::go, &engine);
                bestThread.wait();
                Move bestMove = engine.best_move_last_iter;
                std::cout << "bestmove " << uci::moveToUci(bestMove) << std::endl;
            }
        }
        else if (token == "quit")
        {
            engine.quit();
            break;
        }
    }
}


int main()
{
    testEngine();
    uci_loop();
    return 0;
}
