#include "evaluation.h"
#include "negamax.h"
#include "engine.h"
#include <iostream>
#include <string>
#include <thread>

#include <typeinfo>
#include "play.h"
using namespace chess;

void testCheckmate()
{
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
    Board board = Board(" 8/8/5R2/8/3K1k2/8/3N4/8 b - - 9 5");
    // SimpleEvaluatingFunction modelSimple = SimpleEvaluatingFunction();
    PestoEvaluation modelSimple = PestoEvaluation();
    Evaluation *model = &modelSimple;
    Negamax negamax = Negamax(6, model);
    // https://www.ibm.com/docs/en/rdfi/9.6.0?topic=functions-clock-determine-processor-time
    double time1 = (double)clock() / CLOCKS_PER_SEC;
    play(board, negamax);
    double timedif = (((double)clock()) / CLOCKS_PER_SEC) - time1;
    std::cout << timedif <<std::endl;
    std::cout <<board.getFen();
}

void testEngine(){
    Engine engine = Engine();
    engine.curr_board.get()->setFen("2r1kb2/pp2p2r/2p2n1p/1q1p1pp1/P2P2b1/N5PP/1PPBPPB1/3RKR2 b - - 2 17");
    while (engine.curr_board.get()->isGameOver().first == GameResultReason::NONE){
        engine.go();
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

void testSimple()
{
    Board board = Board();
    SimpleEvaluatingFunction modelSimple = SimpleEvaluatingFunction();
    Evaluation *model = &modelSimple;
    Negamax nega = Negamax(7, model);
    play(board, nega);
}

void testPesto()
{
    Board board = Board();
    PestoEvaluation modelSimple = PestoEvaluation();
    Evaluation *model = &modelSimple;
    Negamax nega = Negamax(6, model);
    play(board, nega);
}

void testBasic()
{
    Board board = Board();
    Negamax nega = Negamax();
    play(board, nega);
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
            if (engine.curr_board.get()->isGameOver().first == GameResultReason::NONE)
            {
                std::thread bestThread = std::thread(&Engine::go, &engine);
                if (bestThread.joinable()){
                    bestThread.join();
                }
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
    //testEngine();
    uci_loop();
    return 0;
}
