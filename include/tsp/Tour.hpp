#pragma once

#include <vector>

#include "tsp/Instance.hpp"

namespace tsp {

// Array-based tour representation: order_[p] = city at position p,
// pos_[c] = position of city c. Supports O(1) successor/predecessor
// queries and O(min(len, n-len)) segment reversal (always reverses
// the shorter arc, which yields an identical resulting cycle).
class Tour {
public:
    explicit Tour(std::vector<int> order);

    int size() const { return n_; }
    int city(int position) const { return order_[position]; }
    int position(int city) const { return pos_[city]; }

    int succ(int city) const { return order_[next(pos_[city])]; }
    int pred(int city) const { return order_[prev(pos_[city])]; }

    // Identifies the exact physical position range a reverseSegment() call
    // touched, so that the move can be undone precisely (see reverseRange).
    struct ReverseHandle {
        int i, j;
    };

    // Reverses the forward arc starting at city a and ending at city b
    // (inclusive), automatically choosing to physically reverse whichever
    // of the two complementary arcs is shorter (the resulting cycle is
    // identical either way). Returns the physical position range that was
    // actually flipped, which callers needing an exact undo should pass to
    // reverseRange() rather than re-deriving a's/b's new positions -- the
    // shorter-side choice can differ between the apply and a naive "swap
    // the arguments" undo call once positions have shifted.
    ReverseHandle reverseSegment(int a, int b);

    // Re-reverses the exact physical position range from a prior
    // reverseSegment() call. Reversing the same range twice is an exact
    // involution, so this is the only safe way to undo a tentative move.
    void reverseRange(ReverseHandle h);

    long length(const Instance& inst) const;

    const std::vector<int>& order() const { return order_; }

    // Replaces the tour with a new city ordering (same multiset of cities).
    void rebuild(std::vector<int> newOrder);

private:
    int next(int p) const { return p + 1 == n_ ? 0 : p + 1; }
    int prev(int p) const { return p == 0 ? n_ - 1 : p - 1; }

    int n_;
    std::vector<int> order_;
    std::vector<int> pos_;
};

}  // namespace tsp
