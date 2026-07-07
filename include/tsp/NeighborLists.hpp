#pragma once

#include <vector>

#include "tsp/Instance.hpp"

namespace tsp {

// k-nearest-neighbor candidate lists, sorted by ascending distance.
// O(n^2) construction, which is fine for instances up to a few thousand
// cities (532 for att532).
class NeighborLists {
public:
    NeighborLists(const Instance& inst, int k);

    const std::vector<int>& neighbors(int city) const { return lists_[city]; }

private:
    std::vector<std::vector<int>> lists_;
};

}  // namespace tsp
