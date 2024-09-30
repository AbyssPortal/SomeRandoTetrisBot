#pragma once

#include <climits>
#include <sstream>
#include <ostream>
#include <iterator>

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
    hold
};

struct MoveInfo {
    bool hold;
    Rotate_State first_rotate;
    int horizontal_move;
    Rotate_State second_rotate;
    MoveInfo() {};
    MoveInfo(const MoveInfo& other) = default;
    MoveInfo& operator=(const MoveInfo& other) = default;
};

struct BotParameters {
    double hole_cost = -100;
    double overhang_cost = -30;
    constexpr static int diff_count = 4;
    double diff_cost[diff_count] = {-0.2, -0.6, -2.4, -9.6};
    double larger_diff_cost = -6;
    constexpr static int setup_count = 3;
    double t_spin_setups_reward[setup_count] = {0, 40, -1};
    double both_setup_reward = 25;
    double too_many_setups_cost = -50;
    double max_height_reward[BOARD_HEIGHT] =
        {0, 2, 5, 10, 15,   /* 5*/
         15, 15, 15, 0, 0,  /* 10*/
         0, -0, -0, -0, -0, /*15*/
         -0, -0, -0, -0, -0 /*20*/};
    double max_height_cost[BOARD_HEIGHT] =  // reward is applied only when not dire, cost is laways applied
        {0, 0, 0, 0, 0,                     /* 5*/
         0, 0, 0, 0, -50,                   /*10*/
         -75, -150, -225, -300, -375,       /*15*/
         -2500, -5000, -6000, -7000, -8000 /*20*/};
    double quad_hole_good_or_bad[BOARD_WIDTH] = {2, -0.5, 0.5, 1.5, 1.25, 1.25, 1.5, 0.5, -0.5, 2};
    double overhang_with_no_t_spin_cost = -20;

    BotParameters() {};

    std::string serialize() const {
        std::ostringstream oss;
        oss << hole_cost << " " << overhang_cost << " ";
        std::copy(std::begin(diff_cost), std::end(diff_cost), std::ostream_iterator<double>(oss, " "));
        oss << larger_diff_cost << " ";
        std::copy(std::begin(t_spin_setups_reward), std::end(t_spin_setups_reward), std::ostream_iterator<double>(oss, " "));
        oss << both_setup_reward << " " << too_many_setups_cost << " ";
        std::copy(std::begin(max_height_reward), std::end(max_height_reward), std::ostream_iterator<double>(oss, " "));
        std::copy(std::begin(max_height_cost), std::end(max_height_cost), std::ostream_iterator<double>(oss, " "));
        std::copy(std::begin(quad_hole_good_or_bad), std::end(quad_hole_good_or_bad), std::ostream_iterator<double>(oss, " "));
        oss << overhang_with_no_t_spin_cost;
        return oss.str();
    }

    static BotParameters deserialize(const std::string& data) {
        std::istringstream iss(data);
        BotParameters params;
        iss >> params.hole_cost >> params.overhang_cost;
        for (int i = 0; i < diff_count; ++i) iss >> params.diff_cost[i];
        iss >> params.larger_diff_cost;
        for (int i = 0; i < setup_count; ++i) iss >> params.t_spin_setups_reward[i];
        iss >> params.both_setup_reward >> params.too_many_setups_cost;
        for (int i = 0; i < BOARD_HEIGHT; ++i) iss >> params.max_height_reward[i];
        for (int i = 0; i < BOARD_HEIGHT; ++i) iss >> params.max_height_cost[i];
        for (int i = 0; i < BOARD_WIDTH; ++i) iss >> params.quad_hole_good_or_bad[i];
        iss >> params.overhang_with_no_t_spin_cost;
        return params;
    }
};


void do_move(const MoveInfo&, StackerGame& game);

void move_block_piece(const MoveInfo& info, const Matrix& game, BlockPiece& piece);

void print_score(const Matrix& mat, const BotParameters& params);

class StackerBot {
   private:
    StackerGame game;
    BotParameters params;

   public:
    StackerBot(const BotParameters& params, int seed) : game(seed), params(params) {};
    StackerGame& get_game();
    MoveInfo suggest_move();
    // returns wether it changed the best
    bool find_moves(Stacker::BlockPiece& pretend_piece, double& curr_best, Stacker::MoveInfo& best_move);
    const BotParameters& get_params() const { return params; };
};

}  // namespace Stacker