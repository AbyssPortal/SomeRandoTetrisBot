
#include "Stacker.h"

using namespace Stacker;

const static int I_PIECE_MINOS[4 /* amount of rots */][4 /*size of i piece*/][2] = {
    {{-2, 0}, {-1, 0}, {0, 0}, {1, 0}},      // zero
    {{0, -2}, {0, -1}, {0, 0}, {0, 1}},      // ninety
    {{-2, -1}, {-1, -1}, {0, -1}, {1, -1}},  // one_eighty
    {{-1, -2}, {-1, -1}, {-1, 0}, {-1, 1}}   // two_seventy
};

const static int T_PIECE_MINOS[4 /* amount of rots */][4 /*size of t piece*/][2] = {
    {{-1, 0}, {0, 0}, {1, 0}, {0, 1}},   // zero
    {{0, -1}, {1, 0}, {0, 0}, {0, 1}},  // ninety
    {{0, -1}, {-1, 0}, {0, 0}, {1, 0}},  // one_eighty
    {{0, -1}, {0, 0}, {-1, 0}, {0, 1}}    // two_seventy
};

const static int S_PIECE_MINOS[4 /* amount of rots */][4 /*size of s piece*/][2] = {
    {{-1, 0}, {0, 0}, {0, 1}, {1, 1}},    // zero
    {{0, 1}, {0, 0}, {1, 0}, {1, -1}},    // ninety
    {{-1, -1}, {0, -1}, {0, 0}, {1, 0}},  // one_eighty
    {{-1, 1}, {-1, 0}, {0, 0}, {0, -1}}   // two_seventy
};

const static int Z_PIECE_MINOS[4 /* amount of rots */][4 /*size of z piece*/][2] = {
    {{-1, 1}, {0, 1}, {0, 0}, {1, 0}},    // zero
    {{0, -1}, {0, 0}, {1, 0}, {1, 1}},    // ninety
    {{-1, 0}, {0, 0}, {0, -1}, {1, -1}},  // one_eighty
    {{-1, -1}, {-1, 0}, {0, 0}, {0, 1}}   // two_seventy
};

const static int J_PIECE_MINOS[4 /* amount of rots */][4 /*size of j piece*/][2] = {
    {{-1, 1}, {-1, 0}, {0, 0}, {1, 0}},  // zero
    {{0, -1}, {0, 0}, {0, 1}, {1, 1}},   // ninety
    {{-1, 0}, {0, 0}, {1, 0}, {1, -1}},  // one_eighty
    {{0, 1}, {0, 0}, {0, -1}, {-1, -1}}  // two_seventy
};

const static int L_PIECE_MINOS[4 /* amount of rots */][4 /*size of l piece*/][2] = {
    {{-1, 0}, {0, 0}, {1, 0}, {1, 1}},    // zero
    {{0, 1}, {0, 0}, {0, -1}, {1, -1}},   // ninety
    {{-1, 0}, {0, 0}, {1, 0}, {-1, -1}},  // one_eighty
    {{-1, 1}, {0, 1}, {0, 0}, {0, -1}}    // two_seventy
};

const static int O_PIECE_MINOS[4 /* amount of rots */][4 /*size of o piece*/][2] = {
    {{-1, 0}, {0, 0}, {-1, 1}, {0, 1}},  // zero
    {{-1, 0}, {0, 0}, {-1, 1}, {0, 1}},  // ninety
    {{-1, 0}, {0, 0}, {-1, 1}, {0, 1}},  // one_eighty
    {{-1, 0}, {0, 0}, {-1, 1}, {0, 1}}   // two_seventy
};

int rotate_to_index(Rotate_State state) {
    return (int(state));
}

const int (*get_piece_minos(Piece_Type type, Rotate_State state))[2] {
    switch (type) {
        case Piece_Type::I:
            return I_PIECE_MINOS[rotate_to_index(state)];
        case Piece_Type::T:
            return T_PIECE_MINOS[rotate_to_index(state)];
        case Piece_Type::S:
            return S_PIECE_MINOS[rotate_to_index(state)];
        case Piece_Type::Z:
            return Z_PIECE_MINOS[rotate_to_index(state)];
        case Piece_Type::J:
            return J_PIECE_MINOS[rotate_to_index(state)];
        case Piece_Type::L:
            return L_PIECE_MINOS[rotate_to_index(state)];
        case Piece_Type::O:
            return O_PIECE_MINOS[rotate_to_index(state)];
        default:
            PANIC("unknown piece type");
    }
}

Rotate_State next_state(Rotate_State state) {
    switch (state) {
        case Rotate_State::zero:
            return Rotate_State::ninety;
        case Rotate_State::ninety:
            return Rotate_State::one_eighty;
        case Rotate_State::one_eighty:
            return Rotate_State::two_seventy;
        case Rotate_State::two_seventy:
            return Rotate_State::zero;
        default:
            PANIC("unkown state");
    }
}

Rotate_State prev_state(Rotate_State state) {
    switch (state) {
        case Rotate_State::zero:
            return Rotate_State::two_seventy;
        case Rotate_State::ninety:
            return Rotate_State::zero;
        case Rotate_State::one_eighty:
            return Rotate_State::ninety;
        case Rotate_State::two_seventy:
            return Rotate_State::one_eighty;
        default:
            PANIC("unknown state");
    }
}

Matrix BlockPiece::location() const {
    Matrix res;

    const int(*offsets)[2] = get_piece_minos(type, state);

    for (int i = 0; i < 4; i++) {
        res.at(x + offsets[i][0], y + offsets[i][1]) = true;
    }
    return res;
};

bool BlockPiece::would_fit(const Matrix& mat, int x, int y) const {
    const int(*offsets)[2] = get_piece_minos(type, state);

    for (int i = 0; i < 4; i++) {
        if (!mat.in_bounds(x + offsets[i][0], y + offsets[i][1])) {
            return false;
        }
        if (mat.at(x + offsets[i][0], y + offsets[i][1]) == true) {
            return false;
        };
    }
    return true;
}

bool BlockPiece::fits(const Matrix& mat) const {
    return would_fit(mat, this->x, this->y);
}

bool BlockPiece::try_offset(const Matrix& mat, int x, int y) {
    this->x += x;
    this->y += y;
    if (fits(mat)) {
        return true;
    } else {
        this->x -= x;
        this->y -= y;
        return false;
    }
}

bool BlockPiece::can_offset(const Matrix& mat, int x, int y) const {
    if (would_fit(mat, this->x + x, this->y + y)) {
        return true;
    } else {
        return false;
    }
};

const static int I_PIECE_SRS_CW[4][5][2] = {
    {{0, 0}, {-2, 0}, {1, 0}, {-2, -1}, {1, 2}},  // zero -> ninety
    {{0, 0}, {-1, 0}, {2, 0}, {1, 2}, {2, -1}},   // ninety -> one eighty
    {{0, 0}, {2, 0}, {-1, 0}, {2, 1}, {-1, -2}},  // one_eighty -> two seventy
    {{0, 0}, {1, 0}, {-2, 0}, {1, -2}, {-2, 1}},  // one_eighty -> two seventy
};
const static int OTHER_PIECE_SRS_CW[4][5][2] = {
    {{0, 0}, {-1, 0}, {-1, 1}, {0, -2}, {-1, -2}},  // zero -> ninety
    {{0, 0}, {1, 0}, {1, -1}, {0, 2}, {1, 2}},      // ninety -> one eighty
    {{0, 0}, {1, 0}, {1, 1}, {0, -2}, {1, -2}},     // one eighty -> two seventy
    {{0, 0}, {-1, 0}, {-1, -1}, {0, 2}, {-1, 2}},   // two seventy -> zero
};

bool BlockPiece::try_90(const Matrix& mat) {
    if (type == Piece_Type::O) {
        return true;  // duh can always spin O
    }
    const int(*offsets)[2];
    if (type == Piece_Type::I) {
        offsets = I_PIECE_SRS_CW[rotate_to_index(this->state)];
    } else {
        offsets = OTHER_PIECE_SRS_CW[rotate_to_index(this->state)];
    };

    this->state = next_state(this->state);

    for (int i = 0; i < 5; i++) {
        x += offsets[i][0];
        y += offsets[i][1];
        if (fits(mat)) {
            return true;
        }
        x -= offsets[i][0];
        y -= offsets[i][1];
    }

    this->state = prev_state(this->state);
    return false;
}

bool BlockPiece::try_270(const Matrix& mat) {
    PANIC("todo");
}

const Matrix& StackerGame::get_board() const {
    return board;
}

const BlockPiece& StackerGame::get_active() const {
    return active_piece;
}

void StackerGame::debug_print() {
    Matrix active = active_piece.location();

    for (int row = BOARD_HEIGHT - 1; row >= 0; row--) {
        std::cout << "[";
        for (int col = 0; col < (int)BOARD_WIDTH; col++) {
            if (active.at(col, row) == true) {
                std::cout << "$ ";
            } else if (board.at(col, row) == true) {
                std::cout << "# ";
            } else {
                std::cout << ". ";
            }
        }
        std::cout << "]" << std::endl;
    }
}

void StackerGame::clear_lines() {
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
}

void StackerGame::tick() {
    while (events.size() > 0) {
        handle_event(events.front());
        events.pop_front();
    }

    check_lock();

    lock_timer.tick();
    gravity.tick();
}

bool Timer::is_running() {
    return running;
}

void StackerGame::check_lock() {
    if (!active_piece.can_offset(board, 0, -1) && !lock_timer.is_running()) {
        lock_timer.set(40 /* random amount*/, false);
    }
}

std::array<Piece_Type, 7> randomize_bag() {
    std::array<Piece_Type, 7> pieces = {
        Piece_Type::I, Piece_Type::J, Piece_Type::L,
        Piece_Type::O, Piece_Type::S, Piece_Type::T, Piece_Type::Z};

    std::shuffle(pieces.begin(), pieces.end(), std::random_device());

    return pieces;
}

StackerGame::StackerGame() : board(), active_piece(Piece_Type::T), events(), next_queue(), empty_hold(true), hold(Piece_Type::T), lock_timer(std::bind(&StackerGame::lock, this)), gravity(std::bind(&StackerGame::drop_one, this)) {
    auto pieces = randomize_bag();
    active_piece = BlockPiece(pieces[0]);
    for (int i = 1; i < 7; i++) {
        next_queue.push_back(pieces[i]);
    }
    gravity.set(30, true);
};

void StackerGame::lock() {
    board |= active_piece.location();
    if (next_queue.size() <= 0) {
        auto pieces = randomize_bag();
        for (int i = 0; i < 7; i++) {
            next_queue.push_back(pieces[i]);
        }
    }
    active_piece = BlockPiece(next_queue.front());
    next_queue.pop_front();
    clear_lines();
};

void StackerGame::handle_event(Event event) {
    switch (event) {
        case Event::tap_left: {
            if (active_piece.try_offset(board, -1, 0)) {
                lock_timer.cancel();
            }
            break;
        }
        case Event::tap_right: {
            if (active_piece.try_offset(board, 1, 0)) {
                lock_timer.cancel();
            }
            break;
        }
        case Event::tap_down: {
            drop_one();
            break;
        }
        case Event::tap_cw: {
            active_piece.try_90(board);
            break;
        }
        case Event::hard_drop: {
            while (active_piece.try_offset(board, 0, -1));
            lock();
            break;
        }
        case Event::hold: {
            if (empty_hold) {
                if (next_queue.size() <= 0) {
                    auto pieces = randomize_bag();
                    for (int i = 0; i < 7; i++) {
                        next_queue.push_back(pieces[i]);
                    }
                }
                hold = active_piece.get_type();
                active_piece = BlockPiece(next_queue.front());
                next_queue.pop_front();
                empty_hold = false;
            } else {
                Piece_Type temp = active_piece.get_type();
                active_piece = BlockPiece(hold);
                hold = temp;
            }
            lock_timer.cancel();
            break;
        }
    }
}

void StackerGame::drop_one() {
    if (active_piece.try_offset(board, 0, -1)) {
        lock_timer.cancel();
    }
}

void StackerGame::send_event(Event event) {
    events.push_back(event);
}

void Timer::set(double period, bool repeat) {
    this->period = period;
    this->repeat = repeat;
    this->accumulated = 0;
    this->running = true;
}

void Timer::cancel() {
    this->running = false;
}

void Timer::tick() {
    if (running) {
        while (this->running && this->accumulated >= period) {
            if (repeat) {
                this->accumulated -= period;
            } else {
                this->running = false;
            }
            func();
        }
        this->accumulated += 1;
    }
}