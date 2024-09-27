#pragma once

#include <deque>
#include <functional>

#include <array>
#include <algorithm>
#include <random>

#include "BitMatrix.h"

namespace Stacker {

const static unsigned int BOARD_HEIGHT = 20;

const static unsigned int BOARD_WIDTH = 10;

typedef BitMatrix<BOARD_WIDTH, BOARD_HEIGHT> Matrix;

class Timer {
    double period;
    double accumulated;
    bool running;
    bool repeat;
    std::function<void()> func;

   public:
    void tick();
    Timer(std::function<void()> function) : period(-1), accumulated(0), running(false), repeat(false),  func(function) {};
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
    BlockPiece(Piece_Type type) : state(Rotate_State::zero), type(type), x(BOARD_WIDTH / 2), y(BOARD_HEIGHT - 2) {};

    bool fits(const Matrix& mat) const;
    // tries to move the given offset (+x = right, +y = down), returns whether succeded.
    virtual bool try_offset(const Matrix& mat, int x, int y);

    // returns whether can in theory do this offset
    virtual bool can_offset(const Matrix& mat, int x, int y) const;

    // tries to rotate clockwise, returns whether succeded.
    bool try_90(const Matrix& mat);
    // tries to rotate ccw, returns whether succeded.
    bool try_270(const Matrix& mat);

    Matrix location() const;

    Piece_Type get_type() const {return type;};

    ~BlockPiece() = default;
};

enum class Event {
    tap_left,
    tap_right,
    tap_down,
    tap_cw,
    tap_ccw,
    hard_drop,
    hold,
};

class StackerGame {
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

    void clear_lines();

    void hard_drop();

    void try_hold();


    Timer gravity;

   public:
    StackerGame();

    void send_event(Event event);

    void tick();

    const Matrix& get_board() const;

    const BlockPiece& get_active() const;

    void debug_print();
};
}  // namespace Stacker