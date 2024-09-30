#include "StackerBot.h"

using namespace Stacker;

// rotates, moves horizontal r/l, and then hard drops
MoveInfo simple_move(int horizontal, Rotate_State state) {
    MoveInfo result;
    result.hold = false;
    result.first_rotate = state;
    result.horizontal_move = horizontal;
    result.second_rotate = Rotate_State::zero;
    return result;
}

// rotates, moves horizontal r/l, soft drops, rotates again, and then hard drops
MoveInfo simple_t_spin(int horizontal, Rotate_State state) {
    MoveInfo result;
    result.hold = false;
    result.first_rotate = state;
    result.horizontal_move = horizontal;
    result.second_rotate = state;

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

struct HoleInformation {
    int hole_count;
    int overhang_count;
};

const static int MAX_OVERHANGS = 1;

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

    for (int col = 1; col < (int)BOARD_WIDTH - 1; col++) {
        int heightL = height_map[col - 1];
        int height = height_map[col];
        int heightR = height_map[col + 1];
        if (height < heightL && height < heightR) {  // check that there's a hole in the bottom
            if (heightR == heightL) {
                continue;  // can't have a t spin setup with even sides
            } else if (heightR < heightL) {
                if (heightR >= BOARD_HEIGHT - 3) {
                    continue;  // too high to have a setup
                }
                if (!mat.at(col - 1, heightR) && mat.at(col - 1, heightR + 1)) {
                    /* look for this shape
                    O

                    O  O
                    O
                    */
                    result[col].t_spin = true;
                }
            } else {
                if (heightL >= BOARD_HEIGHT - 3) {
                    continue;  // too high to have a setup
                }
                if (!mat.at(col + 1, heightL) && mat.at(col + 1, heightL + 1)) {
                    /* look for this shape
                       O

                    O  O
                    O
                    */
                    result[col].t_spin = true;
                }
            }
        }
    }
}

HoleInformation find_holes_and_height_map(const Matrix& mat, int height_map[BOARD_WIDTH]) {
    HoleInformation result;
    result.hole_count = 0;
    result.overhang_count = 0;

    for (int col = 0; col < BOARD_WIDTH; col++) {
        height_map[col] = BOARD_HEIGHT;
        bool sky_light = true;
        for (int row = BOARD_HEIGHT - 1; row >= 0; row--) {
            if (mat.at(col, row)) {
                if (sky_light) {
                    sky_light = false;
                }
            } else {
                if (!sky_light) {
                    result.hole_count++;
                } else {
                    height_map[col] = row;
                }
            }
        }
    }

    for (int col = 0; col < BOARD_WIDTH; col++) {
        for (int row = height_map[col] - 1; row >= 0; row--) {
            if (!mat.at(col, row)) {
                if ((col - 1 > 0 && height_map[col - 1] < row) || (col + 1 <= BOARD_WIDTH && height_map[col + 1] < row)) {
                    result.hole_count--;
                    result.overhang_count++;
                }
            }
        }
    }
    return result;
}

double evaluate_board(const Matrix& mat, int height_map[BOARD_WIDTH], HoleInformation hole_info) {
    const double hole_cost = -100;
    const double overhang_cost = -30;
    const static int diff_count = 4;
    const double diff_cost[diff_count] = {-0.2, -0.6, -2.4, -9.6};
    const double larger_diff_cost = -3;
    const int setup_count = 3;
    const double t_spin_setups_reward[setup_count] = {0, 40, -1};
    const double both_setup_reward = 25;
    const double too_many_setups_cost = -50;
    const double max_height_reward[BOARD_HEIGHT] =
        {0, 1, 2, 3, 15,     /* 5*/
         15, 15, 15, 15, 15, /* 10*/
         0, -0, -0, -0, -0,  /*15*/
         -0, -0, -0, -0, -0 /*20*/};
    const double max_height_cost[BOARD_HEIGHT] =  // reward is applied only when not dire, cost is laways applied
        {0, 0, 0, 0, 0,                           /* 5*/
         0, 0, 0, 0, 0,                           /* 10*/
         0, -100, -150, -300, -450,               /*15*/
         -600, -750, -5000, -10000, -100000 /*20*/};
    const double quad_hole_good_or_bad[BOARD_WIDTH] = {2, -0.5, 0.5, 1.5, 1.25, 1.25, 1.5, 0.5, -0.5, 2};
    const double overhang_with_no_t_spin_cost = -50;

    double value = 0;

    value += hole_cost * hole_info.hole_count;
    value += overhang_cost * hole_info.overhang_count;

    for (int col = 0; col < BOARD_WIDTH - 1; col++) {
        int diff = abs(height_map[col] - height_map[col + 1]);
        if (diff > diff_count - 1) {
            value += larger_diff_cost * diff;
        } else {
            value += diff_cost[diff];
        }
    }
    int max_height = 0;
    for (int col = 0; col < BOARD_WIDTH; col++) {
        if (height_map[col] > max_height) {
            max_height = height_map[col];
        }
    }

    value += max_height_cost[max_height] / 10;

    if (hole_info.hole_count <= 0 && hole_info.overhang_count <= MAX_OVERHANGS) {
        value += max_height_reward[max_height] / 100;

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
            if (t_spin_count == 0) {
                if (hole_info.overhang_count > 0) {
                    value += overhang_with_no_t_spin_cost;
                }
            }
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

double clear_value(const ClearInformation& info) {
    double value = 0;

    if (!info.is_spin) {
        switch (info.clear_count) {
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
                PANICF("somehow cleared %d lines what the sigma\n", info.clear_count);
            }
        }
    } else {
        switch (info.clear_count) {
            case 0: {
                value += 0;
                break;
            }
            case 1: {
                value += 50;
                break;
            }
            case 2: {
                value += 150;
                break;
            }
            case 3: {
                value += 1000;
                break;
            }
            case 4: {
                value += 10000;
                break;
            }
            default: {
                PANICF("somehow cleared %d lines what the sigma\n", info.clear_count);
            }
        }
    }
    return value;
}

ClearInformation clear_lines(Matrix& board, BlockPiece active_piece, bool last_move_was_rotation) {
    int where_to[BOARD_HEIGHT];

    // where_to[0] stores where the 0th line *went*, -1 being removed from existence. the same goes for general i

    int full_so_far = 0;

    // calculate where_to
    for (int row = 0; row < (int)BOARD_HEIGHT; row++) {
        bool full_line = true;
        for (int col = 0; col < (int)BOARD_WIDTH; col++) {
            if (!board.at(col, row)) {
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

    ClearInformation result(active_piece.get_type() == Piece_Type::T && active_piece.three_corners(board) && last_move_was_rotation, full_so_far);

    bool done[BOARD_HEIGHT];
    for (int i = 0; i < (int)BOARD_HEIGHT; i++) {
        done[i] = false;
    }

    // shuffle rows around
    for (int row = 0; row < (int)BOARD_HEIGHT; row++) {
        if (where_to[row] >= 0) {
            for (int col = 0; col < (int)BOARD_WIDTH; col++) {
                board.at(col, where_to[row]) = board.at(col, row);
                done[where_to[row]] = true;
            }
        }
    }
    // fix rows that didnt get nothing
    for (int row = 0; row < (int)BOARD_HEIGHT; row++) {
        if (done[row] == false) {
            for (int col = 0; col < (int)BOARD_WIDTH; col++) {
                board.at(col, row) = false;
            }
        }
    }

    return result;
}

double evaluate_move(Matrix mat, BlockPiece piece, const MoveInfo& info) {
    move_block_piece(info, mat, piece);
    mat |= piece.location();
    bool last_move_was_rotation = (info.second_rotate != Rotate_State::zero);

    ClearInformation clear_info = clear_lines(mat, piece, last_move_was_rotation);
    int height_map[BOARD_WIDTH];
    HoleInformation hole_info = find_holes_and_height_map(mat, height_map);
    double result = evaluate_board(mat, height_map, hole_info);
    if (hole_info.hole_count <= 0 && hole_info.overhang_count <= MAX_OVERHANGS) {
        result += clear_value(clear_info);
    }
    return result;
}

StackerGame& StackerBot::get_game() {
    return game;
}

void do_rotation(const Matrix& mat, BlockPiece& piece, Rotate_State rot) {
    switch (rot) {
        case Rotate_State::zero: {
            break;
        }
        case Rotate_State::ninety: {
            piece.try_90(mat);
            break;
        }
        case Rotate_State::one_eighty: {
            piece.try_180(mat);
            break;
        }
        case Rotate_State::two_seventy: {
            piece.try_270(mat);
            break;
        }
        default: {
            PANICF("Unknown rotation state: %d", static_cast<int>(rot));
        }
    }
}

void Stacker::move_block_piece(const MoveInfo& info, const Matrix& mat, BlockPiece& piece) {
    do_rotation(mat, piece, info.first_rotate);
    if (info.horizontal_move < 0) {
        for (int i = 0; i < -info.horizontal_move; i++) {
            piece.try_offset(mat, -1, 0);
        }
    } else {
        for (int i = 0; i < info.horizontal_move; i++) {
            piece.try_offset(mat, 1, 0);
        }
    }

    while (piece.try_offset(mat, 0, -1));

    do_rotation(mat, piece, info.second_rotate);
}

void Stacker::do_move(const MoveInfo& info, StackerGame& game) {
    if (info.hold) {
        game.send_event(Event::hold);
    }
    switch (info.first_rotate) {
        case Rotate_State::zero:
            break;
        case Rotate_State::ninety:
            game.send_event(Event::tap_cw);
            break;
        case Rotate_State::one_eighty:
            game.send_event(Event::tap_180);
            break;
        case Rotate_State::two_seventy:
            game.send_event(Event::tap_ccw);

            break;
        default:
            PANICF("Unknown rotation state: %d", static_cast<int>(info.first_rotate));
    }

    if (info.horizontal_move < 0) {
        for (int i = 0; i < -info.horizontal_move; i++) {
            game.send_event(Event::press_left);
            game.send_event(Event::release_left);
        }
    } else {
        for (int i = 0; i < info.horizontal_move; i++) {
            game.send_event(Event::press_right);
            game.send_event(Event::release_right);
        }
    }

    if (info.second_rotate != Rotate_State::zero) {
        for (int i = 0; i < BOARD_HEIGHT; i++) {
            game.send_event(Event::press_down);
            game.send_event(Event::release_down);
        }
        switch (info.second_rotate) {
            case Rotate_State::ninety:
                game.send_event(Event::tap_cw);
                break;
            case Rotate_State::one_eighty:
                game.send_event(Event::tap_180);
                break;
            case Rotate_State::two_seventy:
                game.send_event(Event::tap_ccw);
                break;
            default:
                PANICF("Unknown rotation state: %d", static_cast<int>(info.second_rotate));
        }
        game.send_event(Event::hard_drop);

    } else {
        game.send_event(Event::hard_drop);
    }
}

MoveInfo StackerBot::suggest_move() {
    BlockPiece pretend_piece = game.get_active();

    MoveInfo best_move = simple_move(0, Rotate_State::zero);

    double curr_best = -std::numeric_limits<double>::infinity();

    find_moves(pretend_piece, curr_best, best_move);

    if (!game.is_hold_empty()) {
        pretend_piece = BlockPiece(game.get_hold());

    } else {
        pretend_piece = BlockPiece(game.get_next_queue().front());
    }

    if (find_moves(pretend_piece, curr_best, best_move)) {
        best_move.hold = true;
    }

    return best_move;
}

bool Stacker::StackerBot::find_moves(Stacker::BlockPiece& pretend_piece, double& curr_best, Stacker::MoveInfo& best_move) {
    bool res = false;
    Rotate_State states[4] = {Rotate_State::zero, Rotate_State::ninety, Rotate_State::one_eighty, Rotate_State::two_seventy};
    int high_search = BOARD_WIDTH - pretend_piece.get_x();
    int low_search = -pretend_piece.get_x();
    for (auto state : states) {
        for (int i = low_search; i <= high_search; i++) {
            MoveInfo curr_move = simple_move(i, state);
            double eval = evaluate_move(game.get_board(), pretend_piece, curr_move);
            if (eval > curr_best) {
                best_move = curr_move;
                curr_best = eval;
                res = true;
            }
        }
    }
    if (pretend_piece.get_type() == Piece_Type::T) {
        for (int i = low_search; i <= high_search; i++) {
            {
                MoveInfo curr_move = simple_t_spin(i, Rotate_State::ninety);
                double eval = evaluate_move(game.get_board(), pretend_piece, curr_move);
                if (eval > curr_best) {
                    best_move = curr_move;
                    curr_best = eval;
                    res = true;
                }
            }
            {
                MoveInfo curr_move = simple_t_spin(i, Rotate_State::two_seventy);
                double eval = evaluate_move(game.get_board(), pretend_piece, curr_move);
                if (eval > curr_best) {
                    best_move = curr_move;
                    curr_best = eval;
                    res = true;
                }
            }
        }
    }
    return res;
}
