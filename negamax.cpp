#include "negamax.h"
#include "transposition.h"
#include "evaluation.h"
#include <string>
#include <chrono>
#include <iostream>
#include <thread>
#include <condition_variable>
using namespace chess;

//some scores
#define CHECKMATE_SCORE 200000
#define MAXHISTORY INT16_MAX

//score for move ordering
#define BEST_MOVE INT16_MAX
#define MATE_KILLER INT16_MAX/2
#define KILLER_MOVE INT16_MAX/4
#define WORST_ATTACK_SCORE -1000
#define QUIET_MOVE INT16_MIN

//Quiescence depth if enabled
#define QUIESCENCE_DEPTH 4

#define TIME_CHECK 2047

//remove comment for logging
//#define LOGGING

//ENGINE FEATURES

//comment for removing transposition table...
#define TT
//comment for removing alpha-beta pruning
#define PRUNING
//comment for removing move ordering
#define MOVEORDERING 
//comment for removing quiescence
#define QUIESCENCE
// comment for removing IID
#define IID
//comment for removing time...
#define TIMEMOVE
//comment for removing null move
#define NULLMOVE

//for threading
std::mutex history_m;
std::mutex killer_m;
std::mutex best_m;
std::mutex num_nodes_m;
std::condition_variable best_cv;
std::shared_ptr<Score> best_move_th = std::make_shared<Score>();
bool moveFindThread = false;


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
int Negamax::quiescence(Board &board, int alpha, int beta, int quiescence_depth, int ply, int& numNodes){
    #ifdef TIMEMOVE
    if (!(numNodes & TIME_CHECK)){
        if (time_end()){
            is_time_finished.store(true);
        }
    }
    if (interrupt.load() || is_time_finished.load()){
        return 0;
    }
    #endif
    //base cases...
    //check if we find a terminal state...
    //handling checkmates...
    if (isCheckmate(board).first == GameResultReason::CHECKMATE){
        return -CHECKMATE_SCORE + ply; 
    }
    //repeating moves will return 0...
    if (isDraw(board).second == GameResult::DRAW){
        return 0;
    }

    //mate distance pruning
    //MATED IN
    alpha = std::max(alpha, - CHECKMATE_SCORE + ply);
    //MATE IN
    beta = std::min(beta, CHECKMATE_SCORE - ply - 1);
    //prune if mate founded...
    if (alpha >= beta){
        return alpha;
    }

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
        board.makeMove(move);
        ply++;
        {
        std::lock_guard lk(num_nodes_m);
        numNodes++;
        }
        int score = -quiescence( board,-beta, -alpha, quiescence_depth-1, ply, numNodes);
        board.unmakeMove(move);
        ply--;
        if( score >= beta )
            return beta;
        if( score > alpha )
           alpha = score;
    }
    return alpha;
}

void Negamax::moveOrdering(Board &board, Movelist &moves, int local_depth, int& numNodes, int ply)
{
    #ifdef TT
        TTEntry ttEntry = table->lookup(board);
        //Internal Iterative Deepening... if no best move found...
        if (ttEntry.bestMove == Move() && local_depth > 5){
            #ifdef IID
                Move best = this->best(board, local_depth/2, numNodes).move;
                #ifdef LOGGING
                if(best != Move()){
                    std::clog<<"Search IID with depth:" << local_depth/2 <<" completed" <<" nodes examined "<< numNodes<<std::endl;
                }
                #endif
            #endif
        }
    #endif
    for (int i = 0; i < moves.size(); i++)
    {
        #ifdef TT
        //best move
        if (moves[i] == ttEntry.bestMove){
            moves[i].setScore(BEST_MOVE);
            continue;
        }
        #endif
        //killer moves
        if ((*killer_moves)[ply].first == moves[i] || (*killer_moves)[ply].second == moves[i]){
            moves[i].setScore(KILLER_MOVE);
            continue;
        }
        // attacking moves...
        Piece pieceTo = board.at(moves[i].to());
        if (pieceTo != Piece())
        {
            setScoreAttackingMove(board, moves[i], pieceTo);
            continue;
        }
        #ifdef PRUNING
            std::map<std::string, int>::iterator it = history->find(position(board.sideToMove(), moves[i].from(), moves[i].to()));
            if (it !=history->end()){
                //in this way history moves are after good attack moves...
                moves[i].setScore(-(MAXHISTORY -it->second));
                continue;
            }
        #endif
        //quiet moves equally treated
        moves[i].setScore(QUIET_MOVE);
    }
    std::sort(moves.begin(), moves.end(), [](auto const &a, auto const &b)
         { return a.score() > b.score(); });
    assert((moves[0].score()==BEST_MOVE,"Error best move is not on the front of the list..."));
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
Score Negamax::best(Board& board, int local_depth, int& numNodes)
{
    Movelist moves;
    movegen::legalmoves(moves, board);
    //move_ordering if def
    #ifdef MOVEORDERING
    this->moveOrdering(board, moves, local_depth, numNodes, 0);
    #endif
    int alpha = INT_MIN, beta = INT_MAX, ply = 0, bestEvaluation = INT_MIN;
    Move bestMove = Move();
    for (int i = 0; i < moves.size(); i++){
        board.makeMove(moves[i]);
        {
        std::lock_guard lk(num_nodes_m);
        numNodes++;
        }
        ply++;
        int evaluate = -best_priv(board, local_depth-1, alpha, beta, numNodes, ply, moves[i].score() != BEST_MOVE);
        if (evaluate > bestEvaluation){
            bestMove = moves[i];
            bestEvaluation = evaluate;
        }
        moves[i].setScore(evaluate);
        #ifdef LOGGING
            std::clog<<"EVALUATION OF MOVE: "<< chess::uci::moveToUci(moves[i]) << " Score=" << evaluate<<" at depth= "<<local_depth <<" nodes examined "<< numNodes<<std::endl;
        #endif
        ply--;
        board.unmakeMove(moves[i]);
    }
    //invalidate uncorrect searches...
    if(interrupt.load() || is_time_finished.load()){
        return Score();
    }
    #ifdef TT
        TTEntry ttEntry;
        ttEntry.depth = local_depth;
        ttEntry.value = bestEvaluation;
        ttEntry.bestMove = bestMove;
        ttEntry.age = board.halfMoveClock();
        table->store(board, ttEntry);
    #endif
    Score score;
    score.eval = bestEvaluation;
    score.move = bestMove;
    score.depth = local_depth;
    return score;
}

void Negamax::bestMoveThread(Board board, int local_depth, int j_thread, int& numNodes)
{
    Movelist moves;
    movegen::legalmoves(moves, board);
    //move_ordering if def
    #ifdef MOVEORDERING
    this->moveOrdering(board, moves, local_depth, numNodes,0);
    #endif
    int alpha = INT_MIN, beta = INT_MAX, ply=0, bestEvaluation = INT_MIN;
    Move bestMove = Move();
    for (int i = 0; i < moves.size(); i++){
        board.makeMove(moves[i]);
        {
        std::lock_guard lk(num_nodes_m);
        numNodes++;
        }
        ply++;
        int evaluate = -best_priv(board, local_depth-1, alpha, beta, numNodes, ply, moves[i].score() != BEST_MOVE);
        if (evaluate > bestEvaluation){
            bestMove = moves[i];
            bestEvaluation = evaluate;
        }
        moves[i].setScore(evaluate);
        #ifdef LOGGING
            std::clog<<"EVALUATION OF MOVE: "<< chess::uci::moveToUci(moves[i]) << " Score=" << evaluate <<" at depth= "<<local_depth<<" nodes examined "<< numNodes<<std::endl;
        #endif
        ply--;
        board.unmakeMove(moves[i]);
    }
    //invalidate uncorrect searches...
    if(interrupt.load() || is_time_finished.load()){
        {
        std::lock_guard lk(best_m);
        *best_move_th = Score();
        moveFindThread = true;
        }
        best_cv.notify_all();
        return;
    }
    #ifdef TT
        TTEntry ttEntry;
        ttEntry.depth = local_depth;
        ttEntry.value = bestEvaluation;
        ttEntry.bestMove = bestMove;
        ttEntry.age = board.halfMoveClock();
        table->store(board, ttEntry);
    #endif
    Score score;
    score.eval = bestEvaluation;
    score.move = bestMove;
    score.depth = local_depth;
    score.j_thread = j_thread;
    {
    std::lock_guard lk(best_m);
    *best_move_th = score;
    moveFindThread = true;
    }
    best_cv.notify_all();
}



int Negamax::best_priv(Board &board, int local_depth, int alpha, int beta, int& numNodes, int ply, bool can_null)
{
    //break if ending time...)
    #ifdef TIMEMOVE
    if (!(numNodes & TIME_CHECK)){
        if (time_end()){
            is_time_finished.store(true);
        }
    }
    if (interrupt.load() || is_time_finished.load()){
        return 0;
    }
    #endif

    //find if a move is already calculated...
    #if defined(TT) && defined(PRUNING)
        int alphaOrigin = alpha;
        // transposition table check if position already exists...
        TTEntry ttEntry = table->lookup(board);
        if (ttEntry.flag != EMPTY && ttEntry.depth >= local_depth)
        {
            //update the aging factor...
            table->updateAge(board.hash() % TTSIZE, board.halfMoveClock());
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
    #endif


    //base cases...
    //check if we find a terminal state...
    //handling checkmates...
    if (isCheckmate(board).first == GameResultReason::CHECKMATE){
        return -CHECKMATE_SCORE + ply; 
    }
    //repeating moves will return 0...
    if (isDraw(board).second == GameResult::DRAW){
        return 0;
    }
    //if board is in check, we work at higher depth... (like rice engine)...
    if (board.inCheck()){
        local_depth++;
    }
    if (local_depth == 0)
    {
        int value;
        #ifdef QUIESCENCE
        value = this->quiescence(board, alpha, beta, QUIESCENCE_DEPTH, ply, numNodes);
        #endif
        #ifndef QUIESCENCE
        value = this->model->eval(board);
        #endif
        return value;
    }

    //mate distance pruning
    //MATED IN
    alpha = std::max(alpha, - CHECKMATE_SCORE + ply);
    //MATE IN
    beta = std::min(beta, CHECKMATE_SCORE - ply - 1);
    //prune if mate founded...
    if (alpha >= beta){
        return alpha;
    }

    //first try not to move if possible... position must not be in check, at least one piece is required and material advantage must not be enormous...
    #ifdef NULLMOVE
    if (can_null && !board.inCheck() && local_depth > 2 && isThereAMajorPiece(board) && abs(differenceMaterialWhitePerspective(board))<510){
        board.makeNullMove();
        int eval = -best_priv(board, local_depth-2, -beta, -beta+1, numNodes, ply, false);
        board.unmakeNullMove();
        //like rice engine checktime every x nodes...
        #ifdef TIMEMOVE
        if (!(numNodes & TIME_CHECK)){
            if (time_end()){
                is_time_finished.store(true);
            }
        }
        if (interrupt.load() || is_time_finished.load()){
            return 0;
        }
        #endif
        if (eval >= beta){
            //it could be a false mate, so we avoid to return it... (we assume that mate is not bigger than 100 depth...)
            if (eval > CHECKMATE_SCORE - 100){
                eval = beta;
            }
            return eval;
        }
    }
    #endif

    // alpha beta main method...
    int max_value = INT_MIN;
    Move best_move = Move();
    Movelist moves;
    movegen::legalmoves(moves, board);
    //move_ordering if def
    #ifdef MOVEORDERING
    this->moveOrdering(board, moves, local_depth, numNodes, ply);
    #endif
    int value = INT_MIN;
    //finding best move 
    for (int i = 0; i < moves.size(); i++)
    {
        Move move = moves[i];
        board.makeMove(move);
        {
        std::lock_guard lk(num_nodes_m);
        numNodes++;
        }
        ply++;
        //PV SEARCH -> if we are in PV make a complete search
        //https://www.chessprogramming.org/Principal_Variation_Search
        if (move.score() == BEST_MOVE){
            value = -best_priv(board, local_depth - 1, -beta, -alpha, numNodes, ply, false);
        } else {
            value = -best_priv(board, local_depth - 1, -alpha-1, -alpha, numNodes, ply, true);
            //do a research if it is necessary...
            if (value > alpha && beta-alpha > 1){
                value = -best_priv(board, local_depth-1, -beta,-alpha, numNodes, ply, false);
            }
        }
        //update best move
        if (value > max_value){
            best_move = move;
            max_value = value;
        }
        ply--;
        board.unmakeMove(move);
        //update alpha bound... (best move of player alpha)
        alpha = std::max(alpha, max_value);
        #ifdef PRUNING
            //that means that the position must not be explored further...
            if (alpha >= beta)
            {
                //killers
                if (!board.isCapture(move)){
                    //add move to killer moves...
                    updateKillers(ply, move);
                    //https://www.chessprogramming.org/History_Heuristic
                    {
                    std::lock_guard lk(history_m);
                    updateHistory(board, move, ply);
                    for (int j = 0; j < i; j++){
                        //apply a malus for previous quiet move searched...
                        if (!board.isCapture(moves[j])){
                            updateHistory(board, moves[j], - ply);
                        }
                    }
                    }
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
            ttEntry.bestMove = best_move;
        }
        ttEntry.depth = local_depth;
        ttEntry.hash = board.hash();
        ttEntry.age = board.halfMoveClock();
        table->store(board, ttEntry);
    #endif
    return max_value;
}
void Negamax::updateKillers(int ply, const chess::Move &move)
{
    if ((*killer_moves)[ply].first != Move())
    {
        if ((*killer_moves)[ply].second == Move()){
            {
                std::lock_guard lk(killer_m);
                (*killer_moves)[ply].second = move;
            }
        }
        // I could save two killer moves max...
    }
    else if ((*killer_moves)[ply].second != Move())
    {
        {
            std::lock_guard lk(killer_m);
            (*killer_moves)[ply].first = move;
        }
    }
}
void Negamax::updateHistory(chess::Board &board, chess::Move &move, int bonus)
{
    std::map<std::string, int>::iterator it = history->find(position(board.sideToMove(), move.from(), move.to()));
    if (it == history->end())
    {
        (*history)[position(board.sideToMove(), move.from(), move.to())] = std::clamp(bonus, -MAXHISTORY, MAXHISTORY - 1);
    }
    else
    {
        //https://www.chessprogramming.org/History_Heuristic
        int clampedBonus = std::clamp(bonus, -MAXHISTORY, MAXHISTORY - 1);
        (*history)[position(board.sideToMove(), move.from(), move.to())] += clampedBonus - (*history)[position(board.sideToMove(), move.from(), move.to())] * abs(clampedBonus) / MAXHISTORY;
    }
}
Move Negamax::iterative_deepening(Board &board){
    time_start_search = std::chrono::high_resolution_clock::now();
    Score bestMove;
    std::thread threads[num_threads];
    int numNodes = 0;
    //search at depth 1 first...
    bestMove = best(board, 1, numNodes);
    int local_depth = 2;
    int j = 0;
    while(j < num_threads && local_depth < depth ){
        threads[j] = std::thread(bestMoveThread,this, board, local_depth, j, std::ref(numNodes));
        j++;
        local_depth++;
    }
    while(local_depth < depth){
        std::unique_lock lk(best_m);
        best_cv.wait(lk, [] {return moveFindThread;});
        if (best_move_th->move == Move() || interrupt.load() || time_end()) break;  
        int depth_th = best_move_th->depth;
        short j_thread = best_move_th->j_thread;
        if (depth_th > bestMove.depth){
            int score_th = best_move_th->eval;
            Move move_th = best_move_th->move;
            Score newBest;
            newBest.move = move_th;
            newBest.eval = score_th;
            newBest.depth = depth_th;
            bestMove = newBest; 
        }
        *best_move_th = Score();
        moveFindThread = false;
        threads[j_thread].join();
        threads[j_thread] = std::thread(bestMoveThread,this, board, ++local_depth, j_thread, std::ref(numNodes));
        lk.unlock();
        best_cv.notify_all();
    }
    for (int i = 0; i < num_threads; i++){
        if (threads[i].joinable()){
            threads[i].join();
        }
    }
    //clear killer moves
    init_killer(false);
    is_time_finished.store(false);
    *best_move_th = Score();
    moveFindThread = false;
    return bestMove.move;
    }

bool Negamax::isBestMoveMate(chess::Board &board, const chess::Move &best_move_until_now)
{
    board.makeMove(best_move_until_now);
    if (isCheckmate(board).first == GameResultReason::CHECKMATE)
    {
        board.unmakeMove(best_move_until_now);
        return true;
    }
    board.unmakeMove(best_move_until_now);
    return false;
}

bool Negamax::time_end(){
    return (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - time_start_search) > std::chrono::milliseconds(time_move_ms));
}

bool Negamax::isThereAMajorPiece(Board &board){
    uint64_t player_stm_bitboard = 0;
    for (uint8_t piece = int(PieceType::KNIGHT); piece < int(PieceType::KING); piece++){
        PieceType p = PieceType(chess::PieceType::underlying(piece));
        player_stm_bitboard = player_stm_bitboard ^ board.pieces(p,board.sideToMove()).getBits();
    }
    return player_stm_bitboard != 0;
}

int Negamax::differenceMaterialWhitePerspective(Board &board){
    //https://tanjim131.github.io/2020-06-12-count-number-of-1s/
    int white_eval = 0;
    int black_eval = 0;
    
    for (uint8_t piece = int(PieceType::PAWN); piece < int(PieceType::KING); piece++){
        PieceType p = PieceType(chess::PieceType::underlying(piece));
        white_eval += std::bitset<64>(board.pieces(p,Color::WHITE).getBits()).count() * piecesEval[piece];
        black_eval += std::bitset<64>(board.pieces(p,Color::BLACK).getBits()).count() * piecesEval[piece];
    }
    return white_eval - black_eval; 

}

void Negamax::resetTT(){
    for(int i = 0; i < TTSIZE; i++){
        table->tt[i] = TTEntry();
    }
    table->num_elements = 0;
}

std::pair<GameResultReason, GameResult> Negamax::isDraw(Board& board){
    if (board.isHalfMoveDraw()) {
        return board.getHalfMoveDrawType();
    }
    if (board.isInsufficientMaterial()) return {GameResultReason::INSUFFICIENT_MATERIAL, GameResult::DRAW};
    if (board.isRepetition(1)) return {GameResultReason::THREEFOLD_REPETITION, GameResult::DRAW};
    Movelist movelist;
    movegen::legalmoves(movelist, board);
    if (movelist.empty()) {
        return {GameResultReason::STALEMATE, GameResult::DRAW};
    }
    return {GameResultReason::NONE, GameResult::NONE};
}

std::pair<GameResultReason, GameResult> Negamax::isCheckmate(Board &board){
    Movelist movelist;
    movegen::legalmoves(movelist, board);
    if (movelist.empty()) {
        if (board.inCheck()) return {GameResultReason::CHECKMATE, GameResult::LOSE};
    }
    return {GameResultReason::NONE, GameResult::NONE};
}

void Negamax::init_killer(bool reset){
    if (reset) killer_moves = std::make_shared<std::array<std::pair<Move, Move>, 128>>();
    for(int i =0; i < depth; i++){
        (*killer_moves)[i] = std::make_pair(Move(),Move());
    }
}
