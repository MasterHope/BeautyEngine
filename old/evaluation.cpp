#include "chess-library-master/include/chess.hpp"
#include <iostream>
#include "negamax.h"
#include "evaluation.h"
using namespace chess;

int Evaluation::eval(Board &board){
    int i = 0;
        int total_white = 0;
        int total_black = 0;
        while (i < 64){
            Square square = Square(i);
            Piece piece = board.at(square);
            if (piece!=Piece()){
                Color color = piece.color();
                if (color==Color::BLACK)
                    total_black+=piecesEval[int(piece.type())];
                else
                    total_white+=piecesEval[int(piece.type())];
            }
            i++;
        }
        return (board.sideToMove() == Color::BLACK) ? total_black-total_white : total_white-total_black; 
}


int SimpleEvaluatingFunction::eval(Board &board) 
{
    int i = 0;
    int eval_white = 0;
    int eval_black = 0;
    int count_pieces_for_color_no_pawns[2] = {0, 0};
    int whiteKingPosition = -1;
    int blackKingPosition = -1;
    // These are important to define endgame, see: https://www.chessprogramming.org/Simplified_Evaluation_Function
    bool queen_found = false;
    bool queen_found_players[2] = {false, false};
    while (i < 64)
    {
        Square square = Square(i);
        Piece piece = board.at(square);
        if (piece != Piece())
        {
            Color color = piece.color();
            PieceType pieceType = piece.type();
            if (pieceType == PieceType::QUEEN)
            {
                queen_found = true;
                queen_found_players[int(color)] = true;
            }
            if (pieceType != PieceType::KING)
            {
                // count major pieces from both sides...
                if (pieceType != PieceType::PAWN)
                {
                    count_pieces_for_color_no_pawns[int(color)]++;
                }
                int pieceIndex = int(pieceType);
                if (color == Color::BLACK)
                {
                    int square_flip = square.flip().index();
                    eval_black += this->tables[pieceIndex][square_flip] + piecesEval[pieceIndex];
                }
                else
                {
                    eval_white += this->tables[pieceIndex][i] + piecesEval[pieceIndex];
                }
            }
            else if (color == Color::BLACK)
            {
                blackKingPosition = square.flip().index();
            }
            else
            {
                whiteKingPosition = i;
            }
        }
        i++;
    }
    // endgame definition
    if (!queen_found || (queen_found_players[int(Color::WHITE)] and count_pieces_for_color_no_pawns[int(Color::WHITE)] == 1) 
    || (queen_found_players[int(Color::BLACK)] and count_pieces_for_color_no_pawns[int(Color::BLACK)] == 1))
    {
        updateEvalKing(whiteKingPosition, this->kingEvalEndgame, eval_white);
        updateEvalKing(blackKingPosition, this->kingEvalEndgame, eval_black);
    }
    else
    {
        updateEvalKing(whiteKingPosition, this->kingEvalMidgame, eval_white);
        updateEvalKing(blackKingPosition, this->kingEvalMidgame, eval_black);
    }
    return (board.sideToMove() == Color::BLACK) ? eval_black - eval_white : eval_white - eval_black;
}

void SimpleEvaluatingFunction::updateEvalKing(int kingPosition, int *kingEvalTable, int &eval)
{
    if (kingPosition != -1)
        eval += kingEvalTable[kingPosition];
}
