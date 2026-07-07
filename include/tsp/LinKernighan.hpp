#pragma once

#include <deque>
#include <random>
#include <vector>

#include "tsp/Instance.hpp"
#include "tsp/NeighborLists.hpp"
#include "tsp/Tour.hpp"

namespace tsp {

// Lin-Kernighan local search: sequential edge-exchange moves of variable
// depth (implemented as chained 2-opt reversals with backtracking, which
// is how a k-opt LK move decomposes on an array tour representation),
// combined with Or-opt segment relocation, driven by a don't-look-bit
// active-city queue and k-nearest-neighbor candidate lists.
//
// run() wraps this local search in an iterated local search / "chained LK"
// loop that perturbs local optima with random double-bridge (4-opt) moves.
class LinKernighan {
public:
    LinKernighan(const Instance& inst, const NeighborLists& nl, int maxDepth = 6, int breadth = 5);

    // Local-search a tour to a 2-opt/Or-opt/LK local optimum, in place.
    void optimize(Tour& tour);

    // Iterated local search with double-bridge perturbation, bounded by a
    // wall-clock time budget. Returns the best tour found.
    Tour run(Tour initial, double timeLimitSeconds, unsigned seed, bool verbose = false);

private:
    void activate(int city);
    bool improveCity(Tour& tour, int t1);
    bool lkStep(Tour& tour, int t1, int tLast, long gain, int depth, std::vector<int>& chain,
                std::vector<char>& inChain);
    bool orOptCity(Tour& tour, int c);
    void doubleBridge(Tour& tour, std::mt19937& rng);
    void runToLocalOptimum(Tour& tour);

    const Instance& inst_;
    const NeighborLists& nl_;
    int maxDepth_;
    int breadth_;

    std::vector<char> inQueue_;
    std::deque<int> queue_;
};

}  // namespace tsp
