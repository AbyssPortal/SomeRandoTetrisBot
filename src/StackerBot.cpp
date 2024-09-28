#include "StackerBot.h"

using namespace Stacker;

// moves horizontal r/l, and then hard drops
MoveInfo simple_move(int horizontal) {
    MoveInfo result;
    result.move.clear();
    if (horizontal >= 0) {
        for (int j = 0; j < horizontal; j++) {
            result.move.push_back(MovePart::right);
        }
    } else {
        for (int j = 0; j < -horizontal; j++) {
            result.move.push_back(MovePart::left);
        }
    }
    result.move.push_back(MovePart::hard_drop);
    return result;
}

StackerGame& StackerBot::get_game() {
    return game;
}

MoveInfo StackerBot::suggest_move() {
    BlockPiece pretend_piece = game.get_active();

    MoveInfo best_move;

    for (int i = -10; i < 10; i++) {
        BlockPiece curr_piece = pretend_piece;

        if (curr_piece.try_offset(game.get_board(), i, 0) == false) {
            continue;
        }
        return simple_move(i);
    }
}
