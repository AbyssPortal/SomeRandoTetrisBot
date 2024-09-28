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

double evaluate_board(const Matrix& mat) {
    const double hole_cost = -5;
    const static int diff_count = 4;
    const double diff_cost[diff_count] = {-0.1, -0.3, -0.6, -1};
    const double larger_diff_cost = -2;
    double value = 0;
    int height_map[BOARD_WIDTH] = {0};
    for (int col = 0; col < BOARD_WIDTH; col++) {
        bool sky_light = true;
        for (int row = BOARD_HEIGHT - 1; row >= 0; row--) {
            if (mat.at(col, row)) {
                if (sky_light) {
                    sky_light = false;
                }
            } else {
                if (!sky_light) {
                    value += hole_cost;
                } else {
                    height_map[col] = row;
                }
            }
        }
    }
    for (int col = 0; col < BOARD_WIDTH - 1; col++) {
        int diff = abs(height_map[col] - height_map[col + 1]);
        if (diff > diff_count - 1) {
            value += larger_diff_cost * diff;
        } else {
            value += diff_cost[diff];
        }
    }
    return value;
}

double evaluate_move(Matrix mat, BlockPiece piece, const MoveInfo& info) {
    move_block_piece(info, mat, piece);
    mat |= piece.location();
    return evaluate_board(mat);
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
                std::cerr << "dumb shit" << std::endl;
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
