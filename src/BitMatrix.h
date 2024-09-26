#pragma once

#include <bitset>

#include "Macros.h"

template <unsigned int N, unsigned int M>
class BitMatrix {
    std::bitset<N * M> bits;

   public:
    bool at(int i, int j) const;
    static bool in_bounds(int i, int j);
    typename std::bitset<N * M>::reference at(int i, int j);
    BitMatrix();
    ~BitMatrix() = default;

    // Bitwise operators
    BitMatrix<N, M> operator|(const BitMatrix<N, M>& other) const;
    BitMatrix<N, M>& operator|=(const BitMatrix<N, M>& other);
    BitMatrix<N, M> operator&(const BitMatrix<N, M>& other) const;
    BitMatrix<N, M>& operator&=(const BitMatrix<N, M>& other);
};

template <unsigned int N, unsigned int M>
bool BitMatrix<N, M>::in_bounds(int i, int j) {
    if (i < 0 || i >= (int)N || j < 0 || j >= (int)M) {
        return false;
    }
    return true;
}

template <unsigned int N, unsigned int M>
BitMatrix<N, M>::BitMatrix() : bits(){};

template <unsigned int N, unsigned int M>
bool BitMatrix<N, M>::at(int i, int j) const {
    if (!in_bounds(i, j)) {
        PANIC("out of bounds in bit matrix");
    }
    return bits[i * M + j];
}

template <unsigned int N, unsigned int M>
typename std::bitset<N * M>::reference BitMatrix<N, M>::at(int i, int j) {
    if (!in_bounds(i, j)) {
        PANIC("out of bounds in bit matrix");
    }
    return bits[i * M + j];
}

template <unsigned int N, unsigned int M>
BitMatrix<N, M> BitMatrix<N, M>::operator|(const BitMatrix<N, M>& other) const {
    BitMatrix<N, M> result;
    result.bits = this->bits | other.bits;
    return result;
}

template <unsigned int N, unsigned int M>
BitMatrix<N, M>& BitMatrix<N, M>::operator|=(const BitMatrix<N, M>& other) {
    this->bits |= other.bits;
    return *this;
}

template <unsigned int N, unsigned int M>
BitMatrix<N, M> BitMatrix<N, M>::operator&(const BitMatrix<N, M>& other) const {
    BitMatrix<N, M> result;
    result.bits = this->bits & other.bits;
    return result;
}

template <unsigned int N, unsigned int M>
BitMatrix<N, M>& BitMatrix<N, M>::operator&=(const BitMatrix<N, M>& other) {
    this->bits &= other.bits;
    return *this;
}