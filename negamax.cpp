
#include "negamax.h"
#include "transposition.h"
#include "evaluation.h"
#include <string>
#include <iostream>
using namespace chess;

//remove comment for seeing debugging...
//#define LOGGING
//remove comment for using transposition table...
#define TT
//remove comment for using alpha-beta pruning
//#define PRUNING 
std::string position(Color player, Square square_from, Square square_to){
    std::string pos;
    pos.append(std::string(player));
    pos.append(";");
    pos.append(std::string(square_from));
    pos.append(";");
    pos.append(std::string(square_to));
    return pos;
}

Movelist Negamax::moveOrdering(Board &board, Movelist &moves, int local_depth)
{

    Movelist orderedMoves = Movelist();
    Movelist quietMoves = Movelist();
    std::vector<std::pair<Move, int>> quietMovesHistory;
    std::vector<std::pair<Move, int>> attackingMoves;
    #ifdef TT
        TTEntry ttEntry = table->lookup(board);
        // best move from previous iteration...
        if (ttEntry.bestMove != Move())
        {
            orderedMoves.add(ttEntry.bestMove);
        } 
    #endif
    for (const auto &move : moves)
    {
        #ifdef TT
        if (move == ttEntry.bestMove){
            continue;
        }
        #endif
        //add check moves first...
        board.makeMove(move);
        if (board.inCheck()){
            int eval = board.isGameOver().first == GameResultReason::CHECKMATE ? INT_MAX : INT_MAX - 1;
            attackingMoves.push_back(std::make_pair(move, eval));
            board.unmakeMove(move);
            continue;       
        }
        //reset...
        board.unmakeMove(move);
        // attacking moves...
        Piece pieceTo = board.at(move.to());
        if (pieceTo != Piece())
        {
            int pieceFromIndex = int(board.at(move.from()).type());
            int pieceToIndex = int(pieceTo.type());
            // calculating the value of the attack following MVV-LAA...
            int attacking_value =  piecesEval[pieceToIndex] - piecesEval[pieceFromIndex];
            attackingMoves.push_back(std::make_pair(move, attacking_value));
            continue;
        }
        //return end if not found...
        #ifdef PRUNING
            std::map<std::string, int>::iterator it = history.find(position(board.sideToMove(), move.from(), move.to()));
            if (it !=history.end()){
                quietMovesHistory.push_back(std::make_pair(move, it->second));
                continue;
            }
        #endif
        quietMoves.add(move);
        
    }
    //sorting attacking moves by values...
    sort(attackingMoves.begin(), attackingMoves.end(), [](auto const &a, auto const &b)
         { return a.second > b.second; });
    //add first attacking moves, then other...
    for (const auto &move_value : attackingMoves){
        Move move = move_value.first;
        orderedMoves.add(move);
    }
    //sorting quiet moves by history...
    #ifdef PRUNING
        sort(quietMovesHistory.begin(), quietMovesHistory.end(), [](auto const &a, auto const &b)
            { return a.second > b.second; });
        //add historymoves...
        for (const auto &move_value : quietMovesHistory){
            Move move = move_value.first;
            orderedMoves.add(move);
        }
    #endif
    for (const auto &move : quietMoves){
        orderedMoves.add(move);
    }
    
    return orderedMoves;
}
// negamax with alpha beta pruning, starting with alpha and beta with min and max.
//https://en.wikipedia.org/wiki/Negamax
Move Negamax::best(Board &board, int local_depth)
{
    Movelist moves;
    movegen::legalmoves(moves, board);
    //move_ordering
    moves = this->moveOrdering(board, moves, local_depth);

    int bestEvaluation = INT_MIN;
    int alpha = INT_MIN;
    int beta = INT_MAX;
    Move bestMove = Move();

    for (const auto &move : moves){
        #ifdef LOGGING
            std::clog<<"EVALUATION OF MOVE: "<< chess::uci::moveToUci(move) << " " ;
            if (local_depth!=1){
                std::clog<<std::endl;
            }
        #endif
        board.makeMove(move);
        int evaluate = -best_priv(board, local_depth-1, alpha, beta);
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
    std::pair<GameResultReason, GameResult> reason_result = board.isGameOver();
    //handling checkmates...
    if (reason_result.first == GameResultReason::CHECKMATE){
        #ifdef LOGGING
            std::clog << "Checkmate Detected" << std::endl;
        #endif
        return -piecesEval[int(PieceType::KING)] * 10 + local_depth;
    }
    //if board is in check, we work at higher depth...
    /* if (board.inCheck()){
        local_depth++;
        #ifdef LOGGING
        std::clog<< std::endl;
        #endif
    } */
    //repeating moves will return 0...
    if (reason_result.second == GameResult::DRAW){
        #ifdef LOGGING
            std::clog << "0=DRAW" << std::endl;
        #endif
        return 0;
    }
    if (local_depth == 0)
    {
        int ss_eval = this->model->eval(board); 
        #ifdef LOGGING
            std::clog <<"Score = " << ss_eval << std::endl;
        #endif
        return ss_eval;

    }
    int max_value = INT_MIN;
    Movelist moves;
    movegen::legalmoves(moves, board);
    //move_ordering
    moves = this->moveOrdering(board, moves, local_depth);
    //finding best move 
    for (const auto &move : moves)
    {
        board.makeMove(move);
        #ifdef LOGGING
            std::clog<< std::string(curr_depth - local_depth, '.') << "Move executed:" <<chess::uci::moveToUci(move) << " ";
            if (local_depth!=1){
                std::clog<<std::endl;
            }
        #endif
        int value = -best_priv(board, local_depth - 1, -beta, -alpha);
        max_value = std::max(max_value, value);
        board.unmakeMove(move);
        #ifdef PRUNING
            alpha = std::max(alpha, max_value);
            if (alpha >= beta)
            {
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
    Move best_move_until_now = Move();
    while (curr_depth <= this->depth){
        best_move_until_now = this->best(board, curr_depth);
        #ifdef LOGGING
        std::clog<<"best move: " << chess::uci::moveToUci(best_move_until_now)<< " for searching at depth: " << curr_depth <<std::endl;
        #endif
        curr_depth++;
    }
    curr_depth = 1;
    return best_move_until_now;
}