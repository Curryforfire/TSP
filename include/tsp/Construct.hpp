#pragma once

#include "tsp/Instance.hpp"
#include "tsp/NeighborLists.hpp"
#include "tsp/Tour.hpp"

namespace tsp {

// Greedy-edge construction heuristic: repeatedly add the cheapest
// available candidate edge that keeps every vertex degree <= 2 and
// does not close a sub-cycle. Candidate edges are drawn from the
// k-nearest-neighbor lists; any resulting path fragments are stitched
// together with a nearest-endpoint merge to guarantee a valid tour.
Tour greedyEdgeConstruct(const Instance& inst, const NeighborLists& nl);

}  // namespace tsp
