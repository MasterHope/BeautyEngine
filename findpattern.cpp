#include "findpattern.h"

static int pawns_penalities[3] = {25,25,30};

uint64_t get_file_bitboard(int file) {
    return file < 0 || file > 7? 0 : (1ULL << file) | (1ULL << (file + 8)) | (1ULL << (file + 16)) | (1ULL << (file + 24)) |
           (1ULL << (file + 32)) | (1ULL << (file + 40)) | (1ULL << (file + 48)) | (1ULL << (file + 56));
}

std::array<int,3> pawns_structure(Board& board, Color color){
    std::array<int, 3> pawns_structure = std::array<int, 3>();
    auto occ = board.pieces(PieceType::PAWN, color);
    auto occ_other = board.us(~color);
    for (int file = 0; file < 8; file++){
        auto par_result = occ & get_file_bitboard(file);
        auto par_result_opp = occ_other & get_file_bitboard(file);
        //doubled pawn check
        if (attacks::shift<Direction::NORTH>(par_result) & par_result || attacks::shift<Direction::SOUTH>(par_result) & par_result){
            pawns_structure[DOUBLED]++;
        
        //blocked pawn check
        }  else if (par_result_opp & attacks::shift<Direction::NORTH>(par_result)){
            pawns_structure[BLOCKED]++;
        //isolated pawn
        } else if ( par_result && ((occ & get_file_bitboard(file - 1)) ^ (occ & get_file_bitboard(file + 1))) ){
            pawns_structure[ISOLATED]++;
        }
    }
    return pawns_structure;
}



int8_t pawnsPenalitiesColor(Board& board, Color color){
    std::array<int, 3> pawns = pawns_structure(board, color);
    int8_t penality = 0;
    for (int i = DOUBLED; i <= ISOLATED; i++){
        penality+=pawns[i] * pawns_penalities[i];
    }
    return penality;
}

int8_t pawnsPenalities(Board& board){
    return -pawnsPenalitiesColor(board,board.sideToMove()) + pawnsPenalitiesColor(board,~board.sideToMove());
}
//check the mobility score of the player to play compared to other side...
int8_t mobility(Board& board){
    Movelist moves;
    int us = 0, them = 0;
    movegen::legalmoves(moves, board);
    us = moves.size();
    board.makeNullMove();
    movegen::legalmoves(moves, board);
    them = moves.size();
    board.unmakeNullMove();
    return (us - them) * 10;
}

int8_t kingPawnShield(Board& board, Color color){
    Square kingSq = board.kingSq(color);
    std::string castle = board.getCastleString();
    int8_t final_value = 0;
    if (castle != "- -"){
        if (color == Color::WHITE){
            if (castle.find("K") || castle.find("Q")){
                auto occ = board.pieces(PieceType::PAWN, color);
                final_value = __builtin_popcountll((attacks::king(kingSq) & occ).getBits()); 
            }
        } else {
            if (castle.find("k") || castle.find("q")){
                auto occ = board.pieces(PieceType::PAWN, color);
                final_value = __builtin_popcountll((attacks::king(kingSq) & occ).getBits());
            }
        }
    }
    return final_value * 5;
}


int8_t kingVirtualMobility(Board& board, Color color){
    int16_t attacks_from_other_pieces = 0;
    Square kingSq = board.kingSq(color);
    auto occ = board.occ();
    attacks_from_other_pieces += __builtin_popcountll((attacks::queen(kingSq, occ) & board.pieces(PieceType::PAWN, ~color)).getBits()) * 1;
    attacks_from_other_pieces += __builtin_popcountll((attacks::queen(kingSq, occ) & board.pieces(PieceType::KNIGHT, ~color)).getBits()) * 2;
    attacks_from_other_pieces += __builtin_popcountll((attacks::queen(kingSq, occ) & board.pieces(PieceType::BISHOP, ~color)).getBits()) * 2;
    attacks_from_other_pieces += __builtin_popcountll((attacks::queen(kingSq, occ) & board.pieces(PieceType::ROOK, ~color)).getBits()) * 3;
    attacks_from_other_pieces += __builtin_popcountll((attacks::queen(kingSq, occ) & board.pieces(PieceType::QUEEN, ~color)).getBits()) * 5;
    return attacks_from_other_pieces;
}
//"rn1q1bnr/3k4/8/pP3pPp/p4B1p/N1p5/2p5/R3KBNR b KQ - 0 21"
//"k5n1/6P1/3p4/B2P4/8/1P6/1P6/4K3 w - - 0 1"
//"1k6/8/8/3p4/8/8/8/6K1 w - - 0 1"
//"k7/4P3/2P5/8/8/7P/P5P1/K7 w - - 0 1"
//""k5n1/6P1/3p4/B2P4/8/1P6/1P6/4K3 w - - 0 1""
//"k7/6p1/1n3n2/8/3K4/8/8/3b2q1 w - - 0 1"
/* int main(){
    Board board = Board("k7/4P3/2P5/8/8/7P/P5P1/K7 w - - 0 1");
    std::cout<<pawns_structure(board, Color::WHITE)[0]<<std::endl;
    std::cout<<pawns_structure(board, Color::WHITE)[1]<<std::endl;
    std::cout<<pawns_structure(board, Color::WHITE)[2]<<std::endl;
    board = Board();
    std::cout<<board.getCastleString()<<std::endl;
    return 0;
} */