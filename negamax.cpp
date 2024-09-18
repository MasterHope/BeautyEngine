
#include "negamax.h"
#include "transposition.h"
#include "evaluation.h"
#include <string>
#include <iostream>
using namespace chess;

//score for move ordering
#define CHECKMATE_SCORE 200000
#define BEST_MOVE INT16_MAX
#define CHECKMATE_MOVE INT16_MAX - 1
#define CHECK_MOVE INT16_MAX - 2
#define QUIET_MOVE INT16_MIN

//Quiescience depth if enabled
#define QUIESCIENCE_DEPTH 3


//remove comment for seeing debugging...
//#define LOGGING


//ENGINE FEATURES

//comment for removing transposition table...
#define TT
//comment for removing alpha-beta pruning
#define PRUNING
//comment for removing move ordering
#define MOVEORDERING 
//comment for removing quiescence
//#define QUIESCENCE


std::string position(Color player, Square square_from, Square square_to){
    std::string pos;
    pos.append(std::string(player));
    pos.append(";");
    pos.append(std::string(square_from));
    pos.append(";");
    pos.append(std::string(square_to));
    return pos;
}
//https://www.chessprogramming.org/Quiescence_Search
int Negamax::quiescence(Board &board, int alpha, int beta, int quiescence_depth){
    //Generating only capture moves...
    Movelist moves;
    movegen::legalmoves<movegen::MoveGenType::CAPTURE>(moves, board);
    int stand_pat = this->model->eval(board);
    
    //if it is a quiet position or if the recursion stops, than return standard evaluation.
    if (quiescence_depth == 0 || moves.size() == 0){
        return stand_pat;
    }
    if( stand_pat >= beta )
        return beta;
    if( alpha < stand_pat )
        alpha = stand_pat;
    //ordering captures
    for (int i = 0; i < moves.size(); i++){
        Move move = moves[i];
        Piece pieceTo = board.at(move.to());
        // calculating the value of the attack following MVV-LAA...
        setScoreAttackingMove(board, move, pieceTo);
    }
    std::sort(moves.begin(), moves.end(), [](auto const &a, auto const &b)
         { return a.score() > b.score(); });
    
    for(const auto &move : moves)  {
        //not checking losing moves...
        if (move.score() < 0){
            continue;
        }
        board.makeMove(move);
        ply++;
        int score = -quiescence( board,-beta, -alpha, quiescence_depth-1);
        board.unmakeMove(move);
        ply--;
        if( score >= beta )
            return beta;
        if( score > alpha )
           alpha = score;
    }
    return alpha;
}

void Negamax::moveOrdering(Board &board, Movelist &moves, int local_depth)
{
    #ifdef TT
        TTEntry ttEntry = table->lookup(board);
        // best move from previous iteration...
        if (ttEntry.bestMove != Move()){
            if (auto move = std::find(moves.begin(), moves.end(), ttEntry.bestMove)){
                move->setScore(BEST_MOVE);
            }
        //Internal Iterative Deepening...
        } else if (local_depth > 4){
            Move best = this->best(board, local_depth - 3);
            auto move_found = std::find(moves.begin(), moves.end(), best);
            move_found->setScore(BEST_MOVE);
        }
       
    #endif
    for (int i = 0; i < moves.size(); i++)
    {
        Move move = moves[i];
        #ifdef TT
        if (move == ttEntry.bestMove){
            continue;
        }
        #endif
        //add check moves first...
        board.makeMove(move);
        if (board.inCheck()){
            int eval = board.isGameOver().first == GameResultReason::CHECKMATE ? CHECKMATE_MOVE : CHECK_MOVE;
            move.setScore(eval);
            board.unmakeMove(move);
            continue;       
        }
        //reset...
        board.unmakeMove(move);
        // attacking moves...
        Piece pieceTo = board.at(move.to());
        if (pieceTo != Piece())
        {
            setScoreAttackingMove(board, move, pieceTo);
            continue;
        }
        //return end if not found...
        #ifdef PRUNING
            std::map<std::string, int>::iterator it = history.find(position(board.sideToMove(), move.from(), move.to()));
            if (it !=history.end()){
                move.setScore(it->second);
                continue;
            }
        #endif
        move.setScore(QUIET_MOVE);
    }
    std::sort(moves.begin(), moves.end(), [](auto const &a, auto const &b)
         { return a.score() > b.score(); });
}
void Negamax::setScoreAttackingMove(chess::Board &board, chess::Move &move, chess::Piece &pieceTo)
{
    int pieceFromIndex = int(board.at(move.from()).type());
    int pieceToIndex = int(pieceTo.type());
    // calculating the value of the attack following MVV-LAA...
    int attacking_value = piecesEval[pieceToIndex] - piecesEval[pieceFromIndex];
    move.setScore(attacking_value);
}
// negamax with alpha beta pruning, starting with alpha and beta with min and max.
//https://en.wikipedia.org/wiki/Negamax
Move Negamax::best(Board &board, int local_depth)
{
    Movelist moves;
    movegen::legalmoves(moves, board);
    //move_ordering if def
    #ifdef MOVEORDERING
    this->moveOrdering(board, moves, local_depth);
    #endif
    int bestEvaluation = INT_MIN;
    int alpha = INT_MIN;
    int beta = INT_MAX;
    Move bestMove = Move();

    for (const auto &move : moves){
        //like rice engine, check if stop every 2048 nodes...
        if (numNodes % 2048 == 0){
            if (stop || time_end()){
                return 0;
            }
        }
        #ifdef LOGGING
            std::clog<<"EVALUATION OF MOVE: "<< chess::uci::moveToUci(move) << " " ;
            if (local_depth!=1){
                std::clog<<std::endl;
            }
        #endif
        board.makeMove(move);
        numNodes++;
        ply++;
        int evaluate = -best_priv(board, local_depth-1, alpha, beta);
        ply--;
        board.unmakeMove(move);
        if (evaluate > bestEvaluation){
            bestMove = move;
            bestEvaluation = evaluate;
        }
    }
    #ifdef TT
        TTEntry ttEntry;
        ttEntry.depth = local_depth;
        ttEntry.value = bestEvaluation;
        ttEntry.bestMove = bestMove;
        ttEntry.age = board.halfMoveClock();
        table->store(board, ttEntry);
    #endif
    return bestMove;
}
int Negamax::best_priv(Board &board, int local_depth, int alpha, int beta)
{
    //break if ending time... (look every 2048 nodes like rice engine)
    if (numNodes % 2048 == 0){
            if (stop || time_end()){
                return 0;
            }
        }
    //check if we find a terminal state...
     std::pair<GameResultReason, GameResult> reason_result = board.isGameOver();
    //handling checkmates...
    if (reason_result.first == GameResultReason::CHECKMATE){
        #ifdef LOGGING
            std::clog << "Checkmate Detected at ply:" << ply<< std::endl;
        #endif
        return -CHECKMATE_SCORE + ply; 
    }
    //repeating moves will return 0...
    if (reason_result.second == GameResult::DRAW){
        #ifdef LOGGING
            std::clog << "0=DRAW" << std::endl;
        #endif
        return 0;
    }
    //if board is in check, we work at higher depth... (like rice engine)...
    if (board.inCheck()){
        local_depth++;
        #ifdef LOGGING
        std::clog<< std::endl;
        #endif
    }
    if (local_depth == 0)
    {
        int value;
        #ifdef QUIESCENCE
        value = this->quiescence(board, alpha, beta, QUIESCIENCE_DEPTH);
        #endif
        #ifndef QUIESCENCE
        value = this->model->eval(board);
        #endif
        #ifdef LOGGING
            std::clog <<"Score = " << value << std::endl;
        #endif
        return value;
    }
    //from rice engine: check mates...
    //MATED IN
    alpha = std::max(alpha, - CHECKMATE_SCORE + ply);
    //MATE IN
    beta = std::min(beta, CHECKMATE_SCORE - ply - 1);
    //prune if mate founded...
    if (alpha >= beta){
        return alpha;
    }

    #if defined(TT) && defined(PRUNING)
        int alphaOrigin = alpha;
        // transposition table check if position already exists...
        TTEntry ttEntry = table->lookup(board);
        if (ttEntry.flag != EMPTY and ttEntry.depth >= local_depth)
        {
            //update the aging factor...
            table->tt[board.zobrist() % TTSIZE].age = board.halfMoveClock();
            // restore position
            if (ttEntry.flag == EXACT)
            {
                #ifdef LOGGING
                    std::clog <<"Score restored from transposition table = " << ttEntry.value << std::endl;
                #endif
                return ttEntry.value;
            }
            // restore alpha from LOWERBOUND node
            else if (ttEntry.flag == LOWERBOUND)
            {
                alpha = std::max(alpha, ttEntry.value);
            }
            // restore beta from UPPERBOUND node
            else if (ttEntry.flag == UPPERBOUND)
            {
                beta = std::min(beta, ttEntry.value);
            }
            // if alpha>=beta, than we could stop recursion...
            if (alpha >= beta)
            {
                #ifdef LOGGING
                    std::clog <<"Score restored from transposition table = -> alpha >= beta, cutoff: " << ttEntry.value << std::endl;
                #endif
                return ttEntry.value;
            }  
        }
    #endif
    // alpha beta main method...
    int max_value = INT_MIN;
    Movelist moves;
    movegen::legalmoves(moves, board);
    //move_ordering if def
    #ifdef MOVEORDERING
    this->moveOrdering(board, moves, local_depth);
    #endif
    //finding best move 
    for (const auto &move : moves)
    {
        board.makeMove(move);
        numNodes++;
        #ifdef LOGGING
            std::clog<< std::string(curr_depth - local_depth, '.') << "Move executed:" <<chess::uci::moveToUci(move) << " ";
            if (local_depth!=1){
                std::clog<<std::endl;
            }
        #endif
        ply++;
        int value = -best_priv(board, local_depth - 1, -beta, -alpha);
        max_value = std::max(max_value, value);
        ply--;
        board.unmakeMove(move);
        #ifdef PRUNING
            alpha = std::max(alpha, max_value);
            if (alpha >= beta)
            {
                //https://www.chessprogramming.org/History_Heuristic
                if (!board.isCapture(move)){
                    //https://stackoverflow.com/questions/4527686/how-to-update-stdmap-after-using-the-find-method
                    history[position(board.sideToMove(), move.from(), move.to())] += local_depth *local_depth;
                }
                break;
            }
        #endif
    }

    // transposition table store new node...
    #if defined(TT) && defined(PRUNING)
        ttEntry.value = max_value;
        if (max_value <= alphaOrigin)
        {
            ttEntry.flag = UPPERBOUND;
        }
        else if (max_value >= beta)
        {
            ttEntry.flag = LOWERBOUND;
        }
        else
        {
            ttEntry.flag = EXACT;
        }
        ttEntry.depth = local_depth;
        ttEntry.hash = board.hash();
        ttEntry.age = board.halfMoveClock();
        table->store(board, ttEntry);
    #endif
    return max_value;
}
Move Negamax::iterative_deepening(Board &board){
    time(&time_start_search);
    Move best_move_until_now = Move();
    while (curr_depth <= this->depth){
        if (time_end()){
            break;
        }
        best_move_until_now = this->best(board, curr_depth);
        //check if ply is at 0 as excepted
        assert((this->ply==0, "Ply is not correctly updated"));
        //testing mate
        board.makeMove(best_move_until_now);
        if (board.isGameOver().first == GameResultReason::CHECKMATE){
            board.unmakeMove(best_move_until_now);
            break;
        }
        board.unmakeMove(best_move_until_now);
        //
        #ifdef LOGGING
        std::clog<<"best move: " << chess::uci::moveToUci(best_move_until_now)<< " for searching at depth: " << curr_depth <<std::endl;
        #endif
        curr_depth++;
    }
    curr_depth = 1;
    numNodes = 0;
    return best_move_until_now;
}

bool Negamax::time_end(){
    return (time(NULL) - time_start_search > time_move_seconds);
}