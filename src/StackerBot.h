#pragma once

#include <climits>
#include <iterator>
#include <ostream>
#include <sstream>

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
    static constexpr int BOARD_HEIGHT = 20;
    static constexpr int BOARD_WIDTH = 10;
    static constexpr int DIFF_COUNT = 4;
    static constexpr int SETUPS_COUNT = 3;

    static constexpr int hole_cost_location = 0;
    static constexpr int overhang_cost_location = hole_cost_location + 1;
    static constexpr int diff_cost_location = overhang_cost_location + 1;
    static constexpr int larger_diff_cost_location = diff_cost_location + DIFF_COUNT;
    static constexpr int t_spin_setups_reward_location = larger_diff_cost_location + 1;
    static constexpr int both_setup_reward_location = t_spin_setups_reward_location + SETUPS_COUNT;
    static constexpr int too_many_setups_cost_location = both_setup_reward_location + 1;
    static constexpr int max_height_reward_location = too_many_setups_cost_location + 1;
    static constexpr int max_height_cost_location = max_height_reward_location + BOARD_HEIGHT;
    static constexpr int quad_hole_good_or_bad_location = max_height_cost_location + BOARD_HEIGHT;
    static constexpr int overhang_with_no_t_spin_cost_location = quad_hole_good_or_bad_location + BOARD_WIDTH;
    static constexpr int single_reward_location = overhang_with_no_t_spin_cost_location + 1;
    static constexpr int double_reward_location = single_reward_location + 1;
    static constexpr int triple_reward_location = double_reward_location + 1;
    static constexpr int quad_reward_location = triple_reward_location + 1;
    static constexpr int tss_reward_location = quad_reward_location + 1;
    static constexpr int tsd_reward_location = tss_reward_location + 1;
    static constexpr int tst_reward_location = tsd_reward_location + 1;

    static constexpr int TOTAL_SIZE = tst_reward_location + 1;

    std::array<double, TOTAL_SIZE> data;

    BotParameters() {
        data = {
            -100, -30,                                                                                       // hole_cost, overhang_cost
            -0.2, -0.6, -2.4, -9.6,                                                                          // diff_cost
            -6,                                                                                              // larger_diff_cost
            0, 40, -1,                                                                                       // t_spin_setups_reward
            25,                                                                                              // both_setup_reward
            -50,                                                                                             // too_many_setups_cost
            0, 2, 5, 10, 15, 15, 15, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                                 // max_height_reward
            0, 0, 0, 0, 0, 0, 0, 0, 0, -50, -75, -150, -225, -300, -375, -2500, -5000, -6000, -7000, -8000,  // max_height_cost
            8, -2, 2, 6, 5, 5, 6, 2, -2, 8,                                                                  // quad_hole_good_or_bad
            -20,                                                                                             // overhang_with_no_t_spin_cost
            -1, -3, -6, 96,                                                                                  // single_reward, double_reward, triple_reward, quad_reward
            50, 150, 1000                                                                                    // tss_reward, tsd_reward, tst_reward
        };
    }

    double& hole_cost() { return data[hole_cost_location]; }
    double& overhang_cost() { return data[overhang_cost_location]; }
    double& diff_cost(int index) { return data[diff_cost_location + index]; }
    double& larger_diff_cost() { return data[larger_diff_cost_location]; }
    double& t_spin_setups_reward(int index) { return data[t_spin_setups_reward_location + index]; }
    double& both_setup_reward() { return data[both_setup_reward_location]; }
    double& too_many_setups_cost() { return data[too_many_setups_cost_location]; }
    double& max_height_reward(int index) { return data[max_height_reward_location + index]; }
    double& max_height_cost(int index) { return data[max_height_cost_location + index]; }
    double& quad_hole_good_or_bad(int index) { return data[quad_hole_good_or_bad_location + index]; }
    double& overhang_with_no_t_spin_cost() { return data[overhang_with_no_t_spin_cost_location]; }
    double& single_reward() { return data[single_reward_location]; }
    double& double_reward() { return data[double_reward_location]; }
    double& triple_reward() { return data[triple_reward_location]; }
    double& quad_reward() { return data[quad_reward_location]; }
    double& tss_reward() { return data[tss_reward_location]; }
    double& tsd_reward() { return data[tsd_reward_location]; }
    double& tst_reward() { return data[tst_reward_location]; }

    const double& hole_cost() const { return data[hole_cost_location]; }
    const double& overhang_cost() const { return data[overhang_cost_location]; }
    const double& diff_cost(int index) const { return data[diff_cost_location + index]; }
    const double& larger_diff_cost() const { return data[larger_diff_cost_location]; }
    const double& t_spin_setups_reward(int index) const { return data[t_spin_setups_reward_location + index]; }
    const double& both_setup_reward() const { return data[both_setup_reward_location]; }
    const double& too_many_setups_cost() const { return data[too_many_setups_cost_location]; }
    const double& max_height_reward(int index) const { return data[max_height_reward_location + index]; }
    const double& max_height_cost(int index) const { return data[max_height_cost_location + index]; }
    const double& quad_hole_good_or_bad(int index) const { return data[quad_hole_good_or_bad_location + index]; }
    const double& overhang_with_no_t_spin_cost() const { return data[overhang_with_no_t_spin_cost_location]; }
    const double& single_reward() const { return data[single_reward_location]; }
    const double& double_reward() const { return data[double_reward_location]; }
    const double& triple_reward() const { return data[triple_reward_location]; }
    const double& quad_reward() const { return data[quad_reward_location]; }
    const double& tss_reward() const { return data[tss_reward_location]; }
    const double& tsd_reward() const { return data[tsd_reward_location]; }
    const double& tst_reward() const { return data[tst_reward_location]; }

    std::string serialize() const {
        std::ostringstream oss;
        for (int i = 0; i < TOTAL_SIZE; i++) {
            oss << data[i] << " ";
        }
        return oss.str();
    }

    void deserialize(const std::string& str) {
        std::istringstream iss(str);
        for (int i = 0; i < TOTAL_SIZE; i++) {
            iss >> data[i];
        }
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