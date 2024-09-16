
#include "negamax.h"
#include "transposition.h"
#include "evaluation.h"
#include "chess-library-master/include/chess.hpp"
using namespace chess;
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
/* int Negamax::quiescence(Board &board, int alpha, int beta){
    //stop if I detect a checkmate...
    if (board.isGameOver().first == GameResultReason::CHECKMATE){
        return -piecesEval[int(PieceType::KING)];
    }
    int stand_pat = this->model->eval(board);
    if( stand_pat >= beta )
        return beta;
    if( alpha < stand_pat )
        alpha = stand_pat;
    Movelist moves;
    movegen::legalmoves<movegen::MoveGenType::CAPTURE>(moves, board);
    for(const auto &move : moves )  {
        bool isAttackingMove = board.isCapture(move);
        board.makeMove(move);
        int score = -quiescence(board, -beta, -alpha );
        board.unmakeMove(move);
        if( score >= beta )
            return beta;
        if( score > alpha )
           alpha = score;
    }
    return alpha;
} */
Movelist Negamax::moveOrdering(Board &board, Movelist &moves)
{
    Movelist orderedMoves = Movelist();
    Movelist quietMoves = Movelist();
    std::vector<std::pair<Move, int>> quietMovesHistory;
    std::vector<std::pair<Move, int>> attackingMoves;
    TTEntry ttEntry = table->lookup(board);
    for (const auto &move : moves)
    {
        // best move from previous iteration...
        if (move == ttEntry.bestMove)
        {
            orderedMoves.add(move);
            continue;
        }
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
        std::map<std::string, int>::iterator it = history.find(position(board.sideToMove(), move.from(), move.to()));
        if (it !=history.end()){
            quietMovesHistory.push_back(std::make_pair(move, it->second));
            continue;
        }
        quietMoves.add(move);
    
    }
    //sorting attacking moves by values...
    sort(attackingMoves.begin(), attackingMoves.end(), [](auto const &a, auto const &b)
         { return a.second > b.second; });
    //sorting quiet moves by history...
    sort(quietMovesHistory.begin(), quietMovesHistory.end(), [](auto const &a, auto const &b)
         { return a.second > b.second; });
    //add first attacking moves, then other...
    for (const auto &move_value : attackingMoves){
        Move move = move_value.first;
        orderedMoves.add(move);
    }
    //add historymoves...
    for (const auto &move_value : quietMovesHistory){
        Move move = move_value.first;
        orderedMoves.add(move);
    }
    for (const auto &move : quietMoves){
        orderedMoves.add(move);
    }
    
    return orderedMoves;
}
// negamax with alpha beta pruning, starting with alpha and beta with min and max.
//https://en.wikipedia.org/wiki/Negamax
std::pair<Move, int> Negamax::best(Board &board, int local_depth)
{
    Movelist moves;
    movegen::legalmoves(moves, board);
    //move_ordering
    moves = this->moveOrdering(board, moves);

    int bestEvaluation = INT_MIN;
    int alpha = INT_MIN;
    int beta = INT_MAX;
    Move bestMove = Move();

    for (const auto &move : moves){
        board.makeMove(move);
        int evaluate = -best_priv(board, local_depth-1, alpha, beta);
        board.unmakeMove(move);
        if (evaluate >= bestEvaluation){
            bestMove = move;
            bestEvaluation = evaluate;
        }
    }
    TTEntry ttEntry;
    ttEntry.depth = local_depth;
    ttEntry.value = bestEvaluation;
    ttEntry.isvalid = true;
    ttEntry.bestMove = bestMove;
    table->store(board, ttEntry);
    return std::make_pair(bestMove, bestEvaluation);
}
int Negamax::best_priv(Board &board, int local_depth, int alpha, int beta)
{
    int alphaOrigin = alpha;
    // transposition table check if position already exists...
    TTEntry ttEntry = table->lookup(board);
    if (ttEntry.isvalid and ttEntry.depth >= local_depth)
    {
        // restore position
        if (ttEntry.flag == EXACT)
        {
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
            return ttEntry.value;
        }
    }

    // alpha beta main method...
    std::pair<GameResultReason, GameResult> reason_result = board.isGameOver();
    if (reason_result.first == GameResultReason::CHECKMATE){
        return -piecesEval[int(PieceType::KING)] + local_depth;
    }
    if (reason_result.first != GameResultReason::NONE || local_depth == 0)
    {
        return this->model->eval(board);
    }
    int max_value = INT_MIN;
    Movelist moves;
    movegen::legalmoves(moves, board);
    //move_ordering
    moves = this->moveOrdering(board, moves);
    //finding best move 
    for (const auto &move : moves)
    {
        board.makeMove(move);
        GameResultReason resultReason = board.isGameOver().first;
        //like rice engine, if we are in check, we do a better search...
        if (board.inCheck()){
            local_depth++;
        }
        int value = -best_priv(board, local_depth - 1, -beta, -alpha);
        max_value = std::max(max_value, value);
        alpha = std::max(alpha, max_value);
        board.unmakeMove(move);
        if (alpha >= beta && resultReason!=GameResultReason::CHECKMATE)
        {
            if (!board.isCapture(move)){
                history[position(board.sideToMove(), move.from(), move.to())] += local_depth *local_depth;
            }
            break;
        }
    }
    // transposition table store new node...
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
    ttEntry.isvalid = true;
    ttEntry.hash = board.hash();
    table->store(board, ttEntry);
    return max_value;
}
Move Negamax::iterative_deepening(Board &board){
    int curr_depth = 1;
    std::pair<Move, int> best_move_until_now = std::make_pair(Move(), (INT_MIN + 1));
    while (curr_depth <= this->depth){
        best_move_until_now = this->best(board, curr_depth);
        curr_depth++;
    }
    return best_move_until_now.first;
}