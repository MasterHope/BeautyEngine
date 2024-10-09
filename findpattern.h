#ifndef FINDPATTERN_H
#define FINDPATTERN_H
#include "chess-library-master/include/chess.hpp"
using namespace chess;

uint64_t get_file_bitboard(int file);
std::array<int,3> pawns_structure(Board& board, Color color);
int8_t mobility(Board& board);

#endif