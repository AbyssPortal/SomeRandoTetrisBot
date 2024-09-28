#pragma once

#include "Stacker.h"

namespace Stacker {




enum class MovePart {
    left,
    right,
    down,
    ccw,
    cw,
    one_eighty,
    hard_drop,
};

struct MoveInfo{
    std::deque<MovePart> move;
    MoveInfo() {};
    MoveInfo(const MoveInfo& other) : move(other.move) {}
};

class StackerBot {
   private:
    StackerGame game;
   public:
    StackerBot() : game() {};
    StackerGame& get_game();
    MoveInfo suggest_move();
};



}  // namespace Stacker