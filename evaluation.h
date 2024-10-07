#ifndef EVALUATION_H
#define EVALUATION_H
#define FLIP(sq) ((sq) ^ 56)
#define OTHER(side) ((side) ^ 1)

#include "chess-library-master/include/chess.hpp"

using namespace chess;

struct Score{
    int eval = INT_MIN;
    int depth = 0;
    Move move = Move();
    short j_thread;
};

const int piecesEval[6] = {100, 320, 330, 500, 900, 20000};

class Evaluation
{

public:
    Evaluation() {};
    virtual int eval(Board &board);
};
class SimpleEvaluatingFunction : public Evaluation
{
    // https://www.chessprogramming.org/Simplified_Evaluation_Function
private:
    int pawnEval[64] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    5, 10, 10, -20, -20, 10, 10, 5,
    5, -5, -10, 0, 0, -10, -5, 5,
    0, 0, 0, 20, 20, 0, 0, 0,
    5, 5, 10, 25, 25, 10, 5, 5,
    10, 10, 20, 30, 30, 20, 10, 10,
    50, 50, 50, 50, 50, 50, 50, 50,
    0, 0, 0, 0, 0, 0, 0, 0};
    int knightEval[64] = {
    -50, -40, -30, -30, -30, -30, -40, -50,
    -40, -20, 0, 5, 5, 0, -20, -40,
    -30, 5, 10, 15, 15, 10, 5, -30,
    -30, 0, 15, 20, 20, 15, 0, -30,
    -30, 5, 15, 20, 20, 15, 5, -30,
    -30, 0, 10, 15, 15, 10, 0, -30,
    -40, -20, 0, 0, 0, 0, -20, -40,
    -50, -40, -30, -30, -30, -30, -40, -50};
    int bishopEval[64] = {
    -20, -10, -10, -10, -10, -10, -10, -20,
    -10, 5, 0, 0, 0, 0, 5, -10,
    -10, 10, 10, 10, 10, 10, 10, -10,
    -10, 0, 10, 10, 10, 10, 0, -10,
    -10, 5, 5, 10, 10, 5, 5, -10,
    -10, 0, 5, 10, 10, 5, 0, -10,
    -10, 0, 0, 0, 0, 0, 0, -10,
    -20, -10, -10, -10, -10, -10, -10, -20};
    int rookEval[64] = {
    0, 0, 0, 5, 5, 0, 0, 0,
    -5, 0, 0, 0, 0, 0, 0, -5,
    -5, 0, 0, 0, 0, 0, 0, -5,
    -5, 0, 0, 0, 0, 0, 0, -5,
    -5, 0, 0, 0, 0, 0, 0, -5,
    -5, 0, 0, 0, 0, 0, 0, -5,
    5, 10, 10, 10, 10, 10, 10, 5,
    0, 0, 0, 0, 0, 0, 0, 0};
    int queenEval[64] = {
    -20, -10, -10, -5, -5, -10, -10, -20,
    -10, 0, 5, 0, 0, 5, 0, -10,
    -10, 5, 5, 5, 5, 5, 0, -10,
    -5, 0, 5, 5, 5, 5, 0, -5,
    0, 0, 5, 5, 5, 5, 0, -5,
    -5, 0, 5, 5, 5, 5, 0, -5,
    -10, 0, 0, 0, 0, 0, 0, -10,
    -20, -10, -10, -5, -5, -10, -10, -20};
    int kingEvalEndgame[64] = {
    -50,-30,-30,-30,-30,-30,-30,-50
    -30,-30,  0,  0,  0,  0,-30,-30
    -30,-10, 20, 30, 30, 20,-10,-30
    -30,-10, 30, 40, 40, 30,-10,-30
    -30,-10, 30, 40, 40, 30,-10,-30
    -30,-10, 20, 30, 30, 20,-10,-30
    -30,-20,-10,  0,  0,-10,-20,-30
    -50,-40,-30,-20,-20,-30,-40,-50};
    int kingEvalMidgame[64] = {
    20, 30, 10, 0, 0, 10, 30, 20,
    20, 20, 0, 0, 0, 0, 20, 20,
    -10, -20, -20, -20, -20, -20, -20, -10,
    -20, -30, -30, -40, -40, -30, -30, -20,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30};
    int *tables[5] = {pawnEval, knightEval, bishopEval, rookEval, queenEval};

public:
    int eval(Board &board) override;
private:
    void updateEvalKing(int kingPosition, int* kingEvalTable,int &eval);
};

class PestoEvaluation : public Evaluation{
    public:
        int mg_value[6] = { 82, 337, 365, 477, 1025,  0};
        int eg_value[6] = { 94, 281, 297, 512,  936,  0};

        int mg_pawn_table[64] = {
        0,   0,   0,   0,   0,   0,  0,   0,
        -35,  -1, -20, -23, -15,  24, 38, -22,
        -26,  -4,  -4, -10,   3,   3, 33, -12,
        -27,  -2,  -5,  12,  17,   6, 10, -25,
        -14,  13,   6,  21,  23,  12, 17, -23,
        -6,   7,  26,  31,  65,  56, 25, -20,
        98, 134,  61,  95,  68, 126, 34, -11,
        0,   0,   0,   0,   0,   0,  0,   0,
        };

        int eg_pawn_table[64] = {
        0,   0,   0,   0,   0,   0,   0,   0,
        13,   8,   8,  10,  13,   0,   2,  -7,
        -12,  -3,   8,  10, 13,   3,  -7, -15,
        -6,   3,  13,  19,  7,  10,  -3,  -9,
        32,  24,  13,   5,  -2,   4,  17,  17,
        94, 100,  85,  67,  56,  53,  82,  84,
        178, 173, 158, 134, 147, 132, 165, 187,
        0,   0,   0,   0,   0,   0,   0,   0,
        };

        int mg_knight_table[64] = {
        -105, -21, -58, -33, -17, -28, -19,  -23,
        -29, -53, -12,  -3,  -1,  18, -14,  -19,
        -23,  -9,  12,  10,  19,  17,  25,  -16,
        -13,   4,  16,  13,  28,  19,  21,   -8,
        -9,  17,  19,  53,  37,  69,  18,   22,
        -47,  60,  37,  65,  84, 129,  73,   44,
        -73, -41,  72,  36,  23,  62,   7,  -17,
        -167, -89, -34, -49,  61, -97, -15, -107,
        };

        int eg_knight_table[64] = {
        -29, -51, -23, -15, -22, -18, -50, -64,
        -42, -20, -10,  -5,  -2, -20, -23, -44,
        -23,  -3,  -1,  15,  10,  -3, -20, -22,
        -18,  -6,  16,  25,  16,  17,   4, -18,
        -17,   3,  22,  22,  22,  11,   8, -18,
        -24, -20,  10,   9,  -1,  -9, -19, -41,
        -25,  -8, -25,  -2,  -9, -25, -24, -52,
        -58, -38, -13, -28, -31, -27, -63, -99,
        };

        int mg_bishop_table[64] = {
        -33,  -3, -14, -21, -13, -12, -39, -21,
        4,  15,  16,   0,   7,  21,  33,   1,
        0,  15,  15,  15,  14,  27,  18,  10,
        -6,  13,  13,  26,  34,  12,  10,   4,
        -4,   5,  19,  50,  37,  37,   7,  -2,
        -16,  37,  43,  40,  35,  50,  37,  -2,
        -26,  16, -18, -13,  30,  59,  18, -47,
        -29,   4, -82, -37, -25, -42,   7,  -8,
        };

        int eg_bishop_table[64] = {
        -23,  -9, -23,  -5, -9, -16,  -5, -17,
        -14, -18,  -7,  -1,  4,  -9, -15, -27,
        -12,  -3,   8,  10, 13,   3,  -7, -15,
        -6,   3,  13,  19,  7,  10,  -3,  -9,
        -3,  22,  24,  45,  57,  40,  57,  36,
        -20,   6,   9,  49,  47,  35,  19,   9,
        -17,  20,  32,  41,  58,  25,  30,   0,
        -14, -21, -11,  -8, -7,  -9, -17, -24,
        };

        int mg_rook_table[64] = {
        -19, -13,   1,  17, 16,  7, -37, -26,
        -44, -16, -20,  -9, -1, 11,  -6, -71,
        -45, -25, -16, -17,  3,  0,  -5, -33,
        -36, -26, -12,  -1,  9, -7,   6, -23,
        -24, -11,   7,  26, 24, 35,  -8, -20,
        -5,  19,  26,  36, 17, 45,  61,  16,
        27,  32,  58,  62, 80, 67,  26,  44,
        32,  42,  32,  51, 63,  9,  31,  43,
        };

        int eg_rook_table[64] = {
        -9,  2,  3, -1, -5, -13,   4, -20,
        -6, -6,  0,  2, -9,  -9, -11,  -3,
        -4,  0, -5, -1, -7, -12,  -8, -16,
        3,  5,  8,  4, -5,  -6,  -8, -11,
        7,  7,  7,  5,  4,  -3,  -5,  -3,
        11, 13, 13, 11, -3,   3,   8,   3,
        13, 10, 18, 15, 12,  12,   8,   5,
        13, 10, 18, 15, 12,  12,   8,   5,
        };

        int mg_queen_table[64] = {
        -1, -18,  -9,  10, -15, -25, -31, -50,
        -35,  -8,  11,   2,   8,  15,  -3,   1,
        -14,   2, -11,  -2,  -5,   2,  14,   5,
        -9, -26,  -9, -10,  -2,  -4,   3,  -3,
        -27, -27, -16, -16,  -1,  17,  -2,   1,
        -13, -17,   7,   8,  29,  56,  47,  57,
        -24, -39,  -5,   1, -16,  57,  28,  54,
        -28,   0,  29,  12,  59,  44,  43,  45,
        };

        int eg_queen_table[64] = {
        -33, -28, -22, -43,  -5, -32, -20, -41,
        -22, -23, -30, -16, -16, -23, -36, -32,
        -16, -27,  15,   6,   9,  17,  10,   5,
        -18,  28,  19,  47,  31,  34,  39,  23,
        3,  22,  24,  45,  57,  40,  57,  36,
        -20,   6,   9,  49,  47,  35,  19,   9,
        -17,  20,  32,  41,  58,  25,  30,   0,
        -9,  22,  22,  27,  27,  19,  10,  20,
        };

        int mg_king_table[64] = {
        -15,  36,  12, -54,   8, -28,  24,  14,
        1,   7,  -8, -64, -43, -16,   9,   8,
        -14, -14, -22, -46, -44, -30, -15, -27,
        -49,  -1, -27, -39, -46, -44, -33, -51,
        -17, -20, -12, -27, -30, -25, -14, -36,
        -9,  24,   2, -16, -20,   6,  22, -22,
        29,  -1, -20,  -7,  -8,  -4, -38, -29,
        -65,  23,  16, -15, -56, -34,   2,  13,
        };

        int eg_king_table[64] = {
        -53, -34, -21, -11, -28, -14, -24, -43,
        -27, -11,   4,  13,  14,   4,  -5, -17,
        -19,  -3,  11,  21,  23,  16,   7,  -9,
        -18,  -4,  21,  24,  27,  23,   9, -11,
        -8,  22,  24,  27,  26,  33,  26,   3,
        10,  17,  23,  15,  20,  45,  44,  13,
        -12,  17,  14,  17,  17,  38,  23,  11,
        -74, -35, -18, -18, -11,  15,   4, -17,
        };

        int* mg_pesto_table[6] =
        {
            mg_pawn_table,
            mg_knight_table,
            mg_bishop_table,
            mg_rook_table,
            mg_queen_table,
            mg_king_table
        };

        int* eg_pesto_table[6] =
        {
            eg_pawn_table,
            eg_knight_table,
            eg_bishop_table,
            eg_rook_table,
            eg_queen_table,
            eg_king_table
        };
        //changing gamephaseInc to match the enum type for pieces...

        int gamephaseInc[12] = {0,1,1,2,4,0,0,1,1,2,4,0};
        int mg_table[12][64];
        int eg_table[12][64];

        void init_tables();
        int eval(Board &board) override;
        PestoEvaluation();
};

#endif