#pragma once

#include <algorithm>
#include <array>
#include <deque>
#include <functional>
#include <random>

#include "BitMatrix.h"

namespace Stacker {

const static int BOARD_HEIGHT = 20;

const static int BOARD_WIDTH = 10;

typedef BitMatrix<BOARD_WIDTH, BOARD_HEIGHT> Matrix;

class Timer {
    double period;
    double accumulated;
    bool running;
    bool repeat;
    std::function<void()> func;

   public:
    void tick();
    Timer(std::function<void()> function) : period(-1), accumulated(0), running(false), repeat(false), func(function) {};
    void set(double period, bool repeat);
    void cancel();
    bool is_running();
};

enum class Rotate_State {
    zero = 0,
    ninety = 1,
    one_eighty = 2,
    two_seventy = 3,
};

enum class Piece_Type {
    T,
    J,
    L,
    S,
    Z,
    I,
    O
};

class BlockPiece {
    Rotate_State state;
    Piece_Type type;
    int x, y;

    bool would_fit(const Matrix& mat, int x, int y) const;

   public:
    BlockPiece(Piece_Type type);
    bool fits(const Matrix& mat) const;
    // tries to move the given offset (+x = right, +y = down), returns whether succeded.
    bool try_offset(const Matrix& mat, int x, int y);

    // returns whether can in theory do this offset
    bool can_offset(const Matrix& mat, int x, int y) const;

    // tries to rotate clockwise, returns whether succeded.
    bool try_90(const Matrix& mat);
    // tries to rotate ccw, returns whether succeded.
    bool try_270(const Matrix& mat);
    // tries to rotate 180, returns whether succeded
    bool try_180(const Matrix& mat);

    bool three_corners(const Matrix& mat) const;

    Matrix location() const;

    int get_x() {return x;};

    int get_y() {return y;};


    Piece_Type get_type() const { return type; };

    ~BlockPiece() = default;
};

enum class Event {
    press_down,
    release_down,
    press_right,
    press_left,
    release_right,
    release_left,
    tap_cw,
    tap_ccw,
    tap_180,
    hard_drop,
    hold,
};

struct ClearInformation {
    bool is_spin;
    int clear_count;
    ClearInformation(bool is_spin, int clear_count) : is_spin(is_spin), clear_count(clear_count) {};
};

class StackerGame {
    const static constexpr double DAS_INTERVAL = 5;
    const static constexpr double ARR_INTERVAL = 0.01;
    const static constexpr double SOFT_DROP_INTERVAL = 0.01;

    bool last_move_was_rotation;

    Matrix board;

    BlockPiece active_piece;

    std::deque<Event> events;

    std::deque<Piece_Type> next_queue;

    bool empty_hold;

    Piece_Type hold;

    void handle_event(Event event);

    Timer lock_timer;

    void lock();

    void check_lock();

    void drop_one();

    ClearInformation clear_lines();

    ClearInformation last_clear;

    void hard_drop();

    void try_right();

    void try_left();

    void try_hold();

    void start_left_arr();

    void start_right_arr();

    void regenerate_next();

    Timer gravity;

    Timer soft_drop;

    Timer left_DAS;

    Timer left_ARR;

    Timer right_DAS;

    Timer right_ARR;

    bool lock_last_frame;

   public:
    const static constexpr int NEXT_QUEUE_MIN_SIZE = 5;
    StackerGame();

    void send_event(Event event);

    void tick();

    const Matrix& get_board() const;

    const BlockPiece& get_active() const;

    void debug_print() const;

    const std::deque<Piece_Type>& get_next_queue() const;

    Piece_Type get_hold() const;

    BlockPiece get_ghost() const;

    bool is_hold_empty() const;

    ClearInformation get_last_clear() const;

    void empty_last_clear();

    void reset();

    bool is_dead() const;

    bool get_lock_last_frame() const { return lock_last_frame;};
};

}  // namespace Stacker