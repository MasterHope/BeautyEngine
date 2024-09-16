// https://www.chessprogramming.org/PeSTO%27s_Evaluation_Function
#include "chess-library-master/src/include.hpp"
#include "evaluation.h"
using namespace chess;

PestoEvaluation::PestoEvaluation()
{
    init_tables();
}
/* piece/sq tables */
/* values from Rofchade: http://www.talkchess.com/forum3/viewtopic.php?f=2&t=68311&start=19 */

void PestoEvaluation::init_tables()
{
    int pc, p, sq;
    for (p = int(PieceType::PAWN), pc = int(Piece::WHITEPAWN); p <= int(PieceType::KING); pc++, p++)
    {
        for (sq = 0; sq < 64; sq++)
        {
            mg_table[pc][sq] = mg_value[p] + mg_pesto_table[p][sq];
            eg_table[pc][sq] = eg_value[p] + eg_pesto_table[p][sq];
            mg_table[pc + 6][sq] = mg_value[p] + mg_pesto_table[p][FLIP(sq)];
            eg_table[pc + 6][sq] = eg_value[p] + eg_pesto_table[p][FLIP(sq)];
        }
    }
}

int PestoEvaluation::eval(Board &board)
{
    int mg[2];
    int eg[2];
    int gamePhase = 0;

    mg[int(Color::WHITE)] = 0;
    mg[int(Color::BLACK)] = 0;
    eg[int(Color::WHITE)] = 0;
    eg[int(Color::BLACK)] = 0;

    /* evaluate each piece */
    for (int sq = 0; sq < 64; sq++)
    {
        Piece pc = board.at(Square(sq));
        if (pc != Piece())
        {
            mg[pc.color()] += mg_table[pc][sq];
            eg[pc.color()] += eg_table[pc][sq];
            gamePhase += gamephaseInc[pc];
        }
    }

    /* tapered eval */
    int mgScore = mg[board.sideToMove()] - mg[OTHER(board.sideToMove())];
    int egScore = eg[board.sideToMove()] - eg[OTHER(board.sideToMove())];
    int mgPhase = gamePhase;
    if (mgPhase > 24)
        mgPhase = 24; /* in case of early promotion */
    int egPhase = 24 - mgPhase;
    return int((mgScore * mgPhase + egScore * egPhase) / 24);
}
