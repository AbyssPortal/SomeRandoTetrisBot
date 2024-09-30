#pragma once

#include "Stacker.h"
#include <climits>

namespace Stacker {




enum class MovePart {
    left,
    right,
    down,
    ccw,
    cw,
    one_eighty,
    hard_drop,
    hold
};


struct MoveInfo{
    bool hold;
    Rotate_State first_rotate;
    int horizontal_move;
    Rotate_State second_rotate;
    MoveInfo() {};
    MoveInfo(const MoveInfo& other) = default;
    MoveInfo& operator=(const MoveInfo& other) = default;
};

void do_move(const MoveInfo&, StackerGame& game);

void move_block_piece(const MoveInfo& info, const Matrix& game, BlockPiece& piece);


class StackerBot {
   private:
    StackerGame game;
   public:
    StackerBot() : game() {};
    StackerGame& get_game();
    MoveInfo suggest_move();
    //returns wether it changed the best
    bool find_moves(Stacker::BlockPiece& pretend_piece, double& curr_best, Stacker::MoveInfo& best_move);
};

}  // namespace Stacker