#include "StackerBot.h"

using namespace Stacker;

// rotates, moves horizontal r/l, and then hard drops
MoveInfo simple_move(int horizontal, Rotate_State state) {
    MoveInfo result;
    result.move.clear();
    switch (state) {
        case Rotate_State::zero:
            break;
        case Rotate_State::two_seventy:
            result.move.push_back(MovePart::ccw);
            break;
        case Rotate_State::one_eighty:
            result.move.push_back(MovePart::one_eighty);
            break;
        case Rotate_State::ninety:
            result.move.push_back(MovePart::cw);
            break;

        default:
            break;
    }
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

template <typename T>
T abs(T value) {
    return (value < 0) ? -value : value;
}

template <typename T>
T min(T a, T b) {
    return (a < b) ? a : b;
}

struct Setup {
    bool t_spin;
    bool quad;
    int depth;
    Setup() : t_spin(false), quad(false), depth(0) {};
};

void evaluate_t_spin_and_quad(const Matrix& mat, int height_map[BOARD_WIDTH], Setup result[BOARD_WIDTH]) {
    int min_height = BOARD_HEIGHT;
    int second_min_height = BOARD_HEIGHT;

    for (int i = 0; i < (int)BOARD_WIDTH; i++) {
        if (height_map[i] < min_height) {
            second_min_height = min_height;
            min_height = height_map[i];
        } else if (height_map[i] < second_min_height) {
            second_min_height = height_map[i];
        }
    }

    for (int col = 0; col < (int)BOARD_WIDTH; col++) {
        result[col].quad = result[col].t_spin = false;
        if (height_map[col] == min_height) {
            int diff = second_min_height - min_height;
            if (diff >= 2) {
                result[col].quad = true;
                result[col].depth = diff;
            }
        }
    }
}

int find_holes_and_height_map(const Matrix& mat, int height_map[BOARD_WIDTH]) {
    int holes = 0;
    for (int col = 0; col < BOARD_WIDTH; col++) {
        bool sky_light = true;
        for (int row = BOARD_HEIGHT - 1; row >= 0; row--) {
            if (mat.at(col, row)) {
                if (sky_light) {
                    sky_light = false;
                }
            } else {
                if (!sky_light) {
                    holes++;
                } else {
                    height_map[col] = row;
                }
            }
        }
    }
    return holes;
}

double evaluate_board(const Matrix& mat, int height_map[BOARD_WIDTH], int hole_count) {
    const double hole_cost = -100;
    const static int diff_count = 4;
    const double diff_cost[diff_count] = {-0.2, -0.6, -1.2, -1};
    const double larger_diff_cost = -2;
    const int setup_count = 3;
    const double t_spin_setups_reward[setup_count] = {0, 9, -1};
    const double both_setup_reward = 25;
    const double too_many_setups_cost = -50;
    const double max_height_reward[BOARD_HEIGHT] =
        {0, 1, 2, 3, 4,             /* 5*/
         15, 15, 15, 15, 15,           /* 10*/
         15, 0, -50, -100, -200, /*15*/
         -1000, -1000, -10000, -10000, -10000 /*20*/};
    const double quad_hole_good_or_bad[BOARD_WIDTH] = {2, -0.5, 0.5, 1.5, 1.25, 1.25, 1.5, 0.5, -0.5, 2};

    double value = 0;

    value += hole_cost * hole_count;

    for (int col = 0; col < BOARD_WIDTH - 1; col++) {
        int diff = abs(height_map[col] - height_map[col + 1]);
        if (diff > diff_count - 1) {
            value += larger_diff_cost * diff;
        } else {
            value += diff_cost[diff];
        }
    }
    if (hole_count <= 0) {
        int max_height = 0;
        for (int col = 0; col < BOARD_WIDTH; col++) {
            if (height_map[col] > max_height) {
                max_height = height_map[col];
            }
        }
        value += max_height_reward[max_height] / 10;

        Setup setups[BOARD_WIDTH];
        evaluate_t_spin_and_quad(mat, height_map, setups);

        int quad_count = 0;
        int t_spin_count = 0;
        bool both_setup = false;
        for (int col = 0; col < BOARD_WIDTH; col++) {
            auto& setup = setups[col];
            if (setup.t_spin && setup.quad) {
                both_setup = true;
            }
            if (setup.t_spin) {
                t_spin_count++;
            }
            if (setup.quad) {
                quad_count++;
                value += setup.depth * quad_hole_good_or_bad[col] * 2;
            }
        }
        if (t_spin_count < setup_count) {
            value += t_spin_setups_reward[t_spin_count];
        }
        if (both_setup) {
            value += both_setup_reward;
        }
        if (quad_count + t_spin_count > setup_count) {
            value += too_many_setups_cost;
        }
    }
    return value;
}

double clear_value(Matrix mat, BlockPiece piece, const MoveInfo& info) {
    int full_so_far = 0;
    double value = 0;

    // calculate where_to
    for (int row = 0; row < (int)BOARD_HEIGHT; row++) {
        bool full_line = true;
        for (int col = 0; col < (int)BOARD_WIDTH; col++) {
            if (!mat.at(col, row)) {
                full_line = false;
                break;
            }
        }
        if (full_line) {
            full_so_far++;
        }
    }
    switch (full_so_far) {
        case 0: {
            value += 0;
            break;
        }
        case 1: {
            value += -1;
            break;
        }
        case 2: {
            value += -3;
            break;
        }
        case 3: {
            value += -6;
            break;
        }
        case 4: {
            value += 48;
            break;
        }
        default: {
            PANICF("somehow cleared %d lines what the sigma\n", full_so_far);
        }
    }
    return value;
}

void clear_lines(Matrix& mat) {
    int where_to[BOARD_HEIGHT];

    // where_to[0] stores where the 0th line *went*, -1 being removed from existence. the same goes for general i

    int full_so_far = 0;

    // calculate where_to
    for (int row = 0; row < (int)BOARD_HEIGHT; row++) {
        bool full_line = true;
        for (int col = 0; col < (int)BOARD_WIDTH; col++) {
            if (!mat.at(col, row)) {
                full_line = false;
                break;
            }
        }
        if (full_line) {
            full_so_far++;
            where_to[row] = -1;
        } else {
            where_to[row] = row - full_so_far;
        }
    }

    bool done[BOARD_HEIGHT];
    for (int i = 0; i < (int)BOARD_HEIGHT; i++) {
        done[i] = false;
    }

    // shuffle rows around
    for (int row = 0; row < (int)BOARD_HEIGHT; row++) {
        if (where_to[row] >= 0) {
            for (int col = 0; col < (int)BOARD_WIDTH; col++) {
                mat.at(col, where_to[row]) = mat.at(col, row);
                done[where_to[row]] = true;
            }
        }
    }
    // fix rows that didnt get nothing
    for (int row = 0; row < (int)BOARD_HEIGHT; row++) {
        if (done[row] == false) {
            for (int col = 0; col < (int)BOARD_WIDTH; col++) {
                mat.at(col, row) = false;
            }
        }
    }
}

double evaluate_move(Matrix mat, BlockPiece piece, const MoveInfo& info) {
    move_block_piece(info, mat, piece);
    mat |= piece.location();
    clear_lines(mat);
    int height_map[BOARD_WIDTH];
    int hole_count = find_holes_and_height_map(mat, height_map);
    double result = evaluate_board(mat, height_map, hole_count);
    if (hole_count <= 0) {
        result += clear_value(mat, piece, info);
    }
    return result;
}

StackerGame& StackerBot::get_game() {
    return game;
}

void Stacker::move_block_piece(const MoveInfo& info, const Matrix& mat, BlockPiece& piece) {
    for (auto move_part : info.move) {
        switch (move_part) {
            case Stacker::MovePart::left: {
                piece.try_offset(mat, -1, 0);
                break;
            }
            case Stacker::MovePart::right: {
                piece.try_offset(mat, 1, 0);
                break;
            }
            case Stacker::MovePart::down: {
                piece.try_offset(mat, 0, -1);
                break;
            }
            case Stacker::MovePart::hard_drop: {
                while (piece.try_offset(mat, 0, -1));
                break;
            }
            case Stacker::MovePart::cw: {
                piece.try_90(mat);
                break;
            }
            case Stacker::MovePart::ccw: {
                piece.try_270(mat);
                break;
            }
            case Stacker::MovePart::one_eighty: {
                piece.try_180(mat);
                break;
            }
            case Stacker::MovePart::hold: {
                // std::cerr << "dumb shit" << std::endl;
                break;
            }
            default: {
                PANICF("unkown movepart: %d", (int)(move_part));
            }
        }
    }
}

void Stacker::do_move(const MoveInfo& info, StackerGame& game) {
    for (auto move_part : info.move) {
        switch (move_part) {
            case Stacker::MovePart::left: {
                game.send_event(Stacker::Event::press_left);
                game.send_event(Stacker::Event::release_left);

                break;
            }
            case Stacker::MovePart::right: {
                game.send_event(Stacker::Event::press_right);
                game.send_event(Stacker::Event::release_right);

                break;
            }
            case Stacker::MovePart::down: {
                game.send_event(Stacker::Event::press_down);
                game.send_event(Stacker::Event::release_down);

                break;
            }
            case Stacker::MovePart::hard_drop: {
                game.send_event(Stacker::Event::hard_drop);

                break;
            }
            case Stacker::MovePart::cw: {
                game.send_event(Stacker::Event::tap_cw);

                break;
            }
            case Stacker::MovePart::ccw: {
                game.send_event(Stacker::Event::tap_ccw);

                break;
            }
            case Stacker::MovePart::one_eighty: {
                game.send_event(Stacker::Event::tap_180);

                break;
            }
            case Stacker::MovePart::hold: {
                game.send_event(Stacker::Event::hold);
                break;
            }
            default: {
                PANICF("unkown movepart: %d", (int)(move_part));
            }
        }
    }
}

MoveInfo StackerBot::suggest_move() {
    BlockPiece pretend_piece = game.get_active();

    MoveInfo best_move = simple_move(0, Rotate_State::zero);

    Rotate_State states[4] = {Rotate_State::zero, Rotate_State::ninety, Rotate_State::one_eighty, Rotate_State::two_seventy};

    double curr_best = -std::numeric_limits<double>::infinity();

    for (auto state : states) {
        for (int i = -(int)BOARD_WIDTH; i < (int)BOARD_WIDTH; i++) {
            MoveInfo curr_move = simple_move(i, state);
            double eval = evaluate_move(game.get_board(), pretend_piece, curr_move);
            if (eval > curr_best) {
                best_move = curr_move;
                curr_best = eval;
            }
        }
    }

    if (!game.is_hold_empty()) {
        pretend_piece = BlockPiece(game.get_hold());

    } else {
        pretend_piece = BlockPiece(game.get_next_queue().front());
    }

    for (auto state : states) {
        for (int i = -(int)BOARD_WIDTH; i < (int)BOARD_WIDTH; i++) {
            MoveInfo curr_move = simple_move(i, state);
            double eval = evaluate_move(game.get_board(), pretend_piece, curr_move);
            if (eval > curr_best) {
                curr_move.move.push_front(MovePart::hold);
                best_move = curr_move;
                curr_best = eval;
            }
        }
    }

    return best_move;
}
