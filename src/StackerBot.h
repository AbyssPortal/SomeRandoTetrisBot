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
    std::deque<MovePart> move;
    MoveInfo() {};
    MoveInfo(const MoveInfo& other) : move(other.move) {}
    MoveInfo& operator=(const MoveInfo& other) {
        if (this != &other) {
            move = other.move;
        }
        return *this;
    }
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