
#include "Stacker.h"

#define change_lmwr(value)                                                       \
    do {                                                                         \
        last_move_was_rotation = (value);                                        \
    } while (0)

using namespace Stacker;

const static int I_PIECE_MINOS[4 /* amount of rots */][4 /*size of i piece*/][2] = {
    {{-2, 0}, {-1, 0}, {0, 0}, {1, 0}},      // zero
    {{0, -2}, {0, -1}, {0, 0}, {0, 1}},      // ninety
    {{-2, -1}, {-1, -1}, {0, -1}, {1, -1}},  // one_eighty
    {{-1, -2}, {-1, -1}, {-1, 0}, {-1, 1}}   // two_seventy
};

const static int T_PIECE_MINOS[4 /* amount of rots */][4 /*size of t piece*/][2] = {
    {{-1, 0}, {0, 0}, {1, 0}, {0, 1}},   // zero
    {{0, -1}, {1, 0}, {0, 0}, {0, 1}},   // ninety
    {{0, -1}, {-1, 0}, {0, 0}, {1, 0}},  // one_eighty
    {{0, -1}, {0, 0}, {-1, 0}, {0, 1}}   // two_seventy
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

const static int NUM_SRS_CW = 5;

const static int I_PIECE_SRS_CW[4][NUM_SRS_CW][2] = {
    {{0, 0}, {-2, 0}, {1, 0}, {-2, -1}, {1, 2}},  // zero -> ninety
    {{0, 0}, {-1, 0}, {2, 0}, {1, 2}, {2, -1}},   // ninety -> one eighty
    {{0, 0}, {2, 0}, {-1, 0}, {2, 1}, {-1, -2}},  // one_eighty -> two seventy
    {{0, 0}, {1, 0}, {-2, 0}, {1, -2}, {-2, 1}},  // two seventy -> zero
};
const static int OTHER_PIECE_SRS_CW[4][5][2] = {
    {{0, 0}, {-1, 0}, {-1, 1}, {0, -2}, {-1, -2}},  // zero -> ninety
    {{0, 0}, {1, 0}, {1, -1}, {0, 2}, {1, 2}},      // ninety -> one eighty
    {{0, 0}, {1, 0}, {1, 1}, {0, -2}, {1, -2}},     // one eighty -> two seventy
    {{0, 0}, {-1, 0}, {-1, -1}, {0, 2}, {-1, 2}},   // two seventy -> zero
};

const static int NUM_SRS_CCW = 5;

const static int I_PIECE_SRS_COUNTER_CW[4][NUM_SRS_CCW][2] = {
    {{0, 0}, {-1, 0}, {2, 0}, {-1, 2}, {2, -1}},  // zero -> two seventy
    {{0, 0}, {2, 0}, {-1, 0}, {2, 1}, {-1, -2}},  // ninety -> zero
    {{0, 0}, {1, 0}, {-2, 0}, {1, -2}, {-2, 1}},  // one eighty -> ninety
    {{0, 0}, {-2, 0}, {1, 0}, {-2, -1}, {1, 2}},  // two seventy -> one eighty
};

const static int OTHER_PIECE_SRS_COUNTER_CW[4][5][2] = {
    {{0, 0}, {1, 0}, {1, 1}, {0, -2}, {1, -2}},     // zero -> two seventy
    {{0, 0}, {1, 0}, {1, -1}, {0, 2}, {1, 2}},      // ninety -> zero
    {{0, 0}, {-1, 0}, {-1, 1}, {0, -2}, {-1, -2}},  // one eighty -> ninety
    {{0, 0}, {-1, 0}, {-1, -1}, {0, 2}, {-1, 2}},   // two seventy -> one eighty
};

const static int NUM_SRS_180 = 6;

const static int SRS_180[4][NUM_SRS_180][2] = {
    {{0, 0}, {0, 1}, {1, 1}, {-1, 1}, {1, 0}, {-1, 0}},     // zero -> one eighty
    {{0, 0}, {1, 0}, {1, 2}, {1, 1}, {0, 2}, {0, 1}},       // ninety -> two seventy
    {{0, 0}, {0, -1}, {-1, -1}, {1, -1}, {-1, 0}, {1, 0}},  // one eighty -> zero
    {{0, 0}, {-1, 0}, {-1, 2}, {-1, 1}, {0, 2}, {0, 1}},    // two seventy -> ninety
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

    for (int i = 0; i < NUM_SRS_CW; i++) {
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
    if (type == Piece_Type::O) {
        return true;  // duh can always spin O
    }
    const int(*offsets)[2];
    if (type == Piece_Type::I) {
        offsets = I_PIECE_SRS_COUNTER_CW[rotate_to_index(this->state)];
    } else {
        offsets = OTHER_PIECE_SRS_COUNTER_CW[rotate_to_index(this->state)];
    };

    this->state = prev_state(this->state);

    for (int i = 0; i < NUM_SRS_CCW; i++) {
        x += offsets[i][0];
        y += offsets[i][1];
        if (fits(mat)) {
            return true;
        }
        x -= offsets[i][0];
        y -= offsets[i][1];
    }

    this->state = next_state(this->state);
    return false;
}

bool BlockPiece::try_180(const Matrix& mat) {
    if (type == Piece_Type::O) {
        return true;  // duh can always spin O
    } else if (type == Piece_Type::I) {
        state = next_state(next_state(state));
        if (fits(mat)) {
            return true;
        } else {
            state = next_state(next_state(state));
            return false;
        }
    }
    const int(*offsets)[2] = SRS_180[rotate_to_index(this->state)];

    this->state = next_state(next_state(state));

    for (int i = 0; i < NUM_SRS_180; i++) {
        x += offsets[i][0];
        y += offsets[i][1];
        if (fits(mat)) {
            return true;
        }
        x -= offsets[i][0];
        y -= offsets[i][1];
    }

    this->state = next_state(next_state(state));
    return false;
}

int piece_spawn_location(Piece_Type type) {
    switch (type) {
        case Piece_Type::O: {
            return BOARD_WIDTH / 2;
        }
        case Piece_Type::T: {
            return BOARD_WIDTH / 2 - 1;
        }
        case Piece_Type::J: {
            return BOARD_WIDTH / 2 - 1;
        }
        case Piece_Type::L: {
            return BOARD_WIDTH / 2 - 1;
        }
        case Piece_Type::S: {
            return BOARD_WIDTH / 2 - 1;
        }
        case Piece_Type::Z: {
            return BOARD_WIDTH / 2 - 1;
        }
        case Piece_Type::I: {
            return BOARD_WIDTH / 2;
        }

        default: {
            PANICF("unkown piece type: %d", (int)type);
        }
    }
}

BlockPiece::BlockPiece(Piece_Type type) : state(Rotate_State::zero), type(type), x(piece_spawn_location(type)), y(BOARD_HEIGHT - 2) {};

const Matrix& StackerGame::get_board() const {
    return board;
}

const BlockPiece& StackerGame::get_active() const {
    return active_piece;
}

void StackerGame::debug_print() const {
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

bool BlockPiece::three_corners(const Matrix& mat) const {
    int sum = 0;
    if ((!mat.in_bounds(x + 1, y + 1) || mat.at(x + 1, y + 1))) {
        sum++;
    }
    if ((!mat.in_bounds(x - 1, y + 1) || mat.at(x - 1, y + 1))) {
        sum++;
    }
    if ((!mat.in_bounds(x + 1, y - 1) || mat.at(x + 1, y - 1))) {
        sum++;
    }
    if ((!mat.in_bounds(x - 1, y - 1) || mat.at(x - 1, y - 1))) {
        sum++;
    }
    return sum >= 3;
}

ClearInformation StackerGame::clear_lines() {
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

void StackerGame::tick() {
    while (events.size() > 0) {
        handle_event(events.front());
        events.pop_front();
    }

    check_lock();

    lock_timer.tick();
    gravity.tick();
    soft_drop.tick();
    left_DAS.tick();
    left_ARR.tick();
    right_DAS.tick();
    right_ARR.tick();
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

void StackerGame::start_left_arr() {
    left_ARR.set(ARR_INTERVAL, true);
}

void StackerGame::start_right_arr() {
    right_ARR.set(ARR_INTERVAL, true);
}

StackerGame::StackerGame() : board(),
                             active_piece(Piece_Type::T),
                             events(),
                             next_queue(),
                             empty_hold(true),
                             hold(Piece_Type::T),
                             lock_timer(std::bind(&StackerGame::lock, this)),
                             gravity(std::bind(&StackerGame::drop_one, this)),
                             soft_drop(std::bind(&StackerGame::drop_one, this)),
                             left_DAS(std::bind(&StackerGame::start_left_arr, this)),
                             left_ARR(std::bind(&StackerGame::try_left, this)),
                             right_DAS(std::bind(&StackerGame::start_right_arr, this)),
                             right_ARR(std::bind(&StackerGame::try_right, this)),
                             last_clear(false, 0)

{
    auto pieces = randomize_bag();
    active_piece = BlockPiece(pieces[0]);
    for (int i = 1; i < 7; i++) {
        next_queue.push_back(pieces[i]);
    }
    gravity.set(30, true);
};

void StackerGame::regenerate_next() {
    if (next_queue.size() <= NEXT_QUEUE_MIN_SIZE) {
        auto pieces = randomize_bag();
        for (int i = 0; i < 7; i++) {
            next_queue.push_back(pieces[i]);
        }
    }
}

void StackerGame::lock() {
    board |= active_piece.location();
    last_clear = clear_lines();
    if (next_queue.size() <= NEXT_QUEUE_MIN_SIZE) {
        auto pieces = randomize_bag();
        for (int i = 0; i < 7; i++) {
            next_queue.push_back(pieces[i]);
        }
    }
    active_piece = BlockPiece(next_queue.front());
    next_queue.pop_front();
    change_lmwr(false);
};

void StackerGame::try_left() {
    if (active_piece.try_offset(board, -1, 0)) {
        lock_timer.cancel();
        change_lmwr(false);
    }
}

void StackerGame::try_right() {
    if (active_piece.try_offset(board, 1, 0)) {
        lock_timer.cancel();
        change_lmwr(false);
    }
}

void StackerGame::handle_event(Event event) {
    switch (event) {
        case Event::press_left: {
            try_left();
            left_DAS.set(DAS_INTERVAL, false);
            right_DAS.cancel();
            right_ARR.cancel();
            break;
        }
        case Event::press_right: {
            try_right();
            right_DAS.set(DAS_INTERVAL, false);
            left_DAS.cancel();
            left_ARR.cancel();
            break;
        }
        case Event::press_down: {
            drop_one();
            soft_drop.set(SOFT_DROP_INTERVAL, true);
            break;
        }
        case Event::tap_cw: {
            if (active_piece.try_90(board)) {
                lock_timer.cancel();
                change_lmwr(true);
            }
            break;
        }
        case Event::tap_ccw: {
            if (active_piece.try_270(board)) {
                lock_timer.cancel();
                change_lmwr(true);
            }
            break;
        }
        case Event::tap_180: {
            if (active_piece.try_180(board)) {
                lock_timer.cancel();
                change_lmwr(true);
            }
            break;
        }
        case Event::hard_drop: {
            while (active_piece.try_offset(board, 0, -1)) {
                change_lmwr(false);
            }

            lock();
            lock_timer.cancel();
            break;
        }
        case Event::hold: {
            try_hold();
            break;
        }
        case Event::release_right: {
            right_DAS.cancel();
            right_ARR.cancel();
            break;
        }
        case Event::release_left: {
            left_DAS.cancel();
            left_ARR.cancel();
            break;
        }
        case Event::release_down: {
            soft_drop.cancel();
            break;
        }
        default: {
            PANICF("Unkown event: %d", (int)event);
        }
    }
}

void StackerGame::try_hold() {
    if (empty_hold) {
        regenerate_next();
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
}

void StackerGame::drop_one() {
    if (active_piece.try_offset(board, 0, -1)) {
        lock_timer.cancel();
        change_lmwr(false);
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

const std::deque<Piece_Type>& StackerGame::get_next_queue() const {
    return next_queue;
}

Piece_Type StackerGame::get_hold() const {
    return hold;
}

bool StackerGame::is_hold_empty() const {
    return empty_hold;
}

BlockPiece StackerGame::get_ghost() const {
    BlockPiece res = active_piece;
    while (res.try_offset(board, 0, -1));
    return res;
}

void StackerGame::reset() {
    events.clear();
    lock_timer.cancel();
    gravity.cancel();
    soft_drop.cancel();
    left_DAS.cancel();
    left_ARR.cancel();
    right_DAS.cancel();
    right_ARR.cancel();
    board = Matrix();
    next_queue.clear();
    regenerate_next();
    empty_hold = true;
    active_piece = next_queue.front();
    next_queue.pop_front();
    gravity.set(30, true);
}

ClearInformation StackerGame::get_last_clear() const {
    return last_clear;
}