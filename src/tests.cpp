#include "BitMatrix.h"

void test_bit_matrix() {
    BitMatrix<2, 2> matrix;
    matrix.at(0, 0) = true;
    if (matrix.at(0, 0) != true) {
        PANIC("failed assertion");
    }
    if (matrix.at(0, 1) != false) {
        PANIC("failed assertion");
    }
    std::cout << " done testing bit matrix" << std::endl;
}

int main() {
    test_bit_matrix();
    return 0;
}
