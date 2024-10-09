#ifndef FINDPATTERN_H
#define FINDPATTERN_H
#include "chess-library-master/include/chess.hpp"
using namespace chess;


enum PAWN_PENALITIES{DOUBLED, BLOCKED, ISOLATED};

uint64_t get_file_bitboard(int file);
std::array<int,3> pawns_structure(Board& board, Color color);
int8_t pawnsPenalitiesColor(Board& board, Color color);
int8_t pawnsPenalities(Board& board);
int8_t mobility(Board& board);
int8_t kingPawnShield(Board& board, Color color);
int8_t kingVirtualMobility(Board& board, Color color);

#endif