#include "chess-library-master/include/chess.hpp"

using namespace chess;

int main () {
    //see if legalmoves capture considered in check positions?rnbqkbnr/ppp2p1p/6p1/3pp3/3PPP2/5N2/PPP3PP/RNBQKB1R b KQkq - 1 4
    Board board = Board("rnbqkbnr/ppp2p1p/6p1/3pp3/3PPP2/5N2/PPP3PP/RNBQKB1R b KQkq - 1 4");

    Movelist moves;
    movegen::legalmoves<movegen::MoveGenType::QUIET>(moves, board);

    for (const auto &move : moves) {
        std::cout << uci::moveToUci(move) << std::endl;
    }
    return 0;
}