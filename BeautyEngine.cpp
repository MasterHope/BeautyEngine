#include "evaluation.h"
#include "negamax.h"
#include "engine.h"
#include <iostream>
#include <string>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include "play.h"
#include "findpattern.h"
#include "BeautyEngine.h"

using namespace chess;
//draw problem
//position startpos moves d2d4 g8f6 g1f3 e7e6 b1c3 d7d5 c1g5 b8d7 e2e4 h7h6 g5h4 d5e4 c3e4 a7a5 f1d3 a5a4 e1g1 g7g5 h4g3 f6e4 d3e4 f8d6 g3d6 c7d6 d4d5 e6d5 e4d5 d7f6 d1e2 e8f8 c2c4 f8g7 a1d1 a8a5 b2b4 a4b3 a2b3 b7b5 c4b5 g5g4 f3e1 f6d5 d1d5 h8e8 d5d6 d8d6 e2e8 c8e6 h2h3 d6d5 b5b6 a5a8 e8e7 g4h3 g2h3 g7h7 b3b4 e6h3 f2f3 a8g8 g1f2 d5d2 e7e2 d2d4 e2e3 d4e3 f2e3 h3f1 e3f2 f1b5 e1c2 g8b8 c2e3 b8b6 e3d5 b6g6 d5f4 g6d6 f2e3 b5c6 f4e2 c6e8 e2c3 f7f5 b4b5 h6h5 f3f4 d6e6 e3d4 h7g7 d4d5 e6h6 d5e5 g7f7 e5f5 h6b6 f5g5 e8d7 g5h5 d7b5 c3b5 b6b5 h5h4 f7f6 h4g4 b5b3 f4f5 f6g7 g4g5 g7h8 g5g6 b3d3 g6f7 d3b3 f5f6 b3f3 f7e7 h8h7 f6f7 h7g7 f7f8r f3f8 e7e6 g7h7 e6d6 h7g7 d6e6 g7h7 e6e7 f8f2 e7e6 f2a2 e6f7 a2h2 f7f6 h2e2 f6f7 e2e5 f7f6 e5a5 f6e7 a5a6 e7d7 h7g8 d7c7 a6g6 c7b7 g8f8 b7c8 f8e7 c8c7 e7e8 c7c8 g6b6 c8c7 b6b1 c7d6 b1f1 d6e6 f1f4 e6e5 f4g4 e5f5 g4c4 f5e6 c4b4 e6d6 b4b5 d6e6 b5a5 e6f6 e8d7 f6f7 a5a6 f7g8 a6f6 g8h7 d7e6 h7g7 f6f3 g7g6 f3f5 g6g7 f5f6 g7h8 f6f5 h8g7 f5c5 g7g6 c5c3 g6g5 c3c7 g5g4 c7a7 g4g3 e6d7 g3h4 a7a3 h4g4 d7e6 g4f4 a3c3 f4e4 c3c4 e4e3 e6f7 e3d3 c4h4 d3c3 h4h2 c3c4 h2h4 c4d5 f7g7 d5e6 h4h5 e6e7 h5h3 e7e8 h3e3 e8d7 g7g8


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

void test_see(){
    Evaluation evaluation = PestoEvaluation();
    Negamax negamax = Negamax( &evaluation);
    Board board = Board("3r2k1/pp4p1/2p3pn/2Pp2q1/3PPr2/P4B2/1B3P2/2RQ1R1K w - - 1 29");
    Movelist moves;
    movegen::legalmoves<movegen::MoveGenType::CAPTURE>(moves, board);
    for(int i = 0; i < moves.size();i++){
        int see_eval = negamax.see(board, moves[i].to(), board.sideToMove());
        std::cout<<see_eval<<std::endl;
    }
    
}

void differenceMaterial(){
    Engine engine = Engine();
    engine.curr_board->setFen("8/8/5R2/8/3K1k2/8/3N4/8 b - - 9 5");
    std::cout<<engine.negamax->differenceMaterialWhitePerspective(*engine.curr_board)<<std::endl;
}

void testEngine(){
    Engine engine = Engine();
    while (engine.curr_board->isGameOver().first == GameResultReason::NONE){
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

void go_uci(Engine & engine){
    engine.go();
    {
    std::lock_guard lk(search);
    isSearching = false;
    }
    cv.notify_one();
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
        if (token == "stop" && isSearching){
            {
                std::lock_guard lk(stop);
                engine.negamax->interrupt.store(true);
            }
            //wait to finish...
            {
                std::unique_lock lk(search);
                cv.wait(lk, [] { return !isSearching;});
            }
            //remove stop
            {
                std::lock_guard lk(stop);
                engine.negamax->interrupt.store(false);
            }
        }
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
            std::string line_fen; 
            std::string fen;
            std::string moves;
            std::size_t pos;
            is >> std::skipws >> option;
            if (option == "startpos")
            {
                engine.curr_board->setFen(STARTINGFEN);
                getline(is, line_fen);
                pos = line_fen.find("moves");
                if (pos != std::string::npos){
                    moves = line_fen.substr(pos + 6, line_fen.length() - 1);
                }
            } else if (option == "fen") {
                getline(is, line_fen);
                pos = line_fen.find("moves");
                if (pos != std::string::npos){
                    fen = line_fen.substr(1, pos);
                    moves = line_fen.substr(pos + 6, line_fen.length() - 1);
                } else {
                    fen = line_fen.substr(1, line_fen.length() - 1);
                }
                engine.curr_board->setFen(fen);
            }
            if (notMoves(pos))
            {
                continue;
            }
            std::istringstream movesStream(moves);
            std::string move;
            while (movesStream >> std::skipws >> move)
            {
                engine.curr_board->makeMove(uci::uciToMove(*engine.curr_board,move)); 
            }
        }
        else if (token == "ucinewgame")
        {
            engine.reset();
        }
        else if (token == "go" && !isSearching)
        {
            is >> std::skipws >> token;
            if (token=="movetime"){
                is >> std::skipws >> token;
                engine.setTime(atoi(token.c_str()));
            }
            if(engine.curr_board->isGameOver().second != GameResult::NONE){
                std::cout<<"GAME IS ENDED"<<std::endl;
                continue;
            }
            std::lock_guard lk(search);
            {
            isSearching = true;
            }
            std::thread find_best_move(go_uci, std::ref(engine));
            find_best_move.detach();
        }
        else if (token == "quit")
        {
            if (isSearching){
                {
                    std::lock_guard lk(stop);
                    engine.negamax->interrupt.store(true);
                }
                //wait to finish...
                {
                    std::unique_lock lk(search);
                    cv.wait(lk, [] { return !isSearching;});
                }
                //remove stop
                {
                    std::lock_guard lk(stop);
                    engine.negamax->interrupt.store(false);
                }
            }
            break;
        }
    }
}

bool notMoves(std::size_t pos)
{
    return pos == std::string::npos;
}
int main()
{
    uci_loop();
    return 0;
}

/* int main(){
    Board board = Board("k7/4P3/2P5/8/8/7P/P5P1/K7 w - - 0 1");
    std::cout<<pawns_structure(board, Color::WHITE)[0]<<std::endl;
    std::cout<<pawns_structure(board, Color::WHITE)[1]<<std::endl;
    std::cout<<pawns_structure(board, Color::WHITE)[2]<<std::endl;
    board = Board();
    std::cout<<board.getCastleString()<<std::endl;
    int8_t val = kingPawnShield(board, Color::WHITE); 
    board = Board("k7/6p1/1n3n2/8/3K4/8/8/3b2q1 w - - 0 1");
    int8_t king = kingVirtualMobility(board, Color::WHITE);
    std::cout<<king+1<<std::endl;
    return 0;
}
 */

