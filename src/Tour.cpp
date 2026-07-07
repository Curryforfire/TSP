#include "tsp/Tour.hpp"

#include <algorithm>

namespace tsp {

Tour::Tour(std::vector<int> order) : n_(static_cast<int>(order.size())), order_(std::move(order)), pos_(n_) {
    for (int p = 0; p < n_; ++p) pos_[order_[p]] = p;
}

Tour::ReverseHandle Tour::reverseSegment(int a, int b) {
    int i = pos_[a];
    int j = pos_[b];
    int segLen = j - i;
    if (segLen < 0) segLen += n_;
    segLen += 1;

    if (segLen * 2 > n_) {
        // Reverse the complementary (shorter) arc instead: succ(b) .. pred(a).
        i = next(j);
        j = prev(pos_[a]);
    }

    reverseRange({i, j});
    return {i, j};
}

void Tour::reverseRange(ReverseHandle h) {
    int i = h.i, j = h.j;
    int segLen = j - i;
    if (segLen < 0) segLen += n_;
    segLen += 1;

    for (int k = 0; k < segLen / 2; ++k) {
        int p1 = i + k;
        if (p1 >= n_) p1 -= n_;
        int p2 = j - k;
        if (p2 < 0) p2 += n_;
        std::swap(order_[p1], order_[p2]);
        pos_[order_[p1]] = p1;
        pos_[order_[p2]] = p2;
    }
}

void Tour::rebuild(std::vector<int> newOrder) {
    order_ = std::move(newOrder);
    for (int p = 0; p < n_; ++p) pos_[order_[p]] = p;
}

long Tour::length(const Instance& inst) const {
    long total = 0;
    for (int p = 0; p < n_; ++p) {
        total += inst.dist(order_[p], order_[next(p)]);
    }
    return total;
}

}  // namespace tsp
