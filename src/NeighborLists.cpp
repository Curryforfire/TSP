#include "tsp/NeighborLists.hpp"

#include <algorithm>

namespace tsp {

NeighborLists::NeighborLists(const Instance& inst, int k) {
    int n = inst.size();
    lists_.resize(n);
    std::vector<std::pair<long, int>> candidates;
    candidates.reserve(n - 1);

    for (int c = 0; c < n; ++c) {
        candidates.clear();
        for (int other = 0; other < n; ++other) {
            if (other == c) continue;
            candidates.emplace_back(inst.dist(c, other), other);
        }
        int kk = std::min<int>(k, static_cast<int>(candidates.size()));
        std::partial_sort(candidates.begin(), candidates.begin() + kk, candidates.end());
        lists_[c].reserve(kk);
        for (int i = 0; i < kk; ++i) lists_[c].push_back(candidates[i].second);
    }
}

}  // namespace tsp
