#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <optional>
#include <string>

#include "tsp/Construct.hpp"
#include "tsp/Instance.hpp"
#include "tsp/LinKernighan.hpp"
#include "tsp/NeighborLists.hpp"
#include "tsp/Tour.hpp"

namespace {

bool validateTour(const tsp::Tour& tour) {
    int n = tour.size();
    std::vector<char> seen(n, 0);
    for (int p = 0; p < n; ++p) {
        int c = tour.city(p);
        if (c < 0 || c >= n || seen[c]) return false;
        seen[c] = 1;
    }
    return true;
}

void printUsage(const char* prog) {
    std::fprintf(stderr,
                  "Usage: %s <instance.tsp> [time_budget_seconds] [known_optimal] [seed] [k_neighbors]\n"
                  "  instance.tsp        TSPLIB file (EUC_2D / CEIL_2D / ATT / GEO)\n"
                  "  time_budget_seconds default 20\n"
                  "  known_optimal       optional, prints optimality gap if given\n"
                  "  seed                default 1\n"
                  "  k_neighbors         default 8\n",
                  prog);
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    std::string path = argv[1];
    double timeBudget = argc > 2 ? std::atof(argv[2]) : 20.0;
    std::optional<long> knownOptimal;
    if (argc > 3) knownOptimal = std::atol(argv[3]);
    unsigned seed = argc > 4 ? static_cast<unsigned>(std::atoi(argv[4])) : 1u;
    int k = argc > 5 ? std::atoi(argv[5]) : 8;

    tsp::Instance inst;
    try {
        inst = tsp::Instance::readFile(path);
    } catch (const std::exception& e) {
        std::fprintf(stderr, "Failed to read instance: %s\n", e.what());
        return 1;
    }

    std::printf("Instance: %s (n=%d)\n", inst.name().c_str(), inst.size());

    using clock = std::chrono::steady_clock;
    auto t0 = clock::now();
    auto since = [&](clock::time_point t) { return std::chrono::duration<double>(clock::now() - t).count(); };

    tsp::NeighborLists nl(inst, k);
    std::printf("Built %d-nearest-neighbor candidate lists in %.2fs\n", k, since(t0));

    auto t1 = clock::now();
    tsp::Tour tour = tsp::greedyEdgeConstruct(inst, nl);
    long constructLen = tour.length(inst);
    std::printf("Greedy-edge construction: length=%ld  (%.2fs)\n", constructLen, since(t1));

    tsp::LinKernighan lk(inst, nl, /*maxDepth=*/6, /*breadth=*/5);

    auto t2 = clock::now();
    tsp::Tour best = lk.run(tour, timeBudget, seed, /*verbose=*/true);
    long bestLen = best.length(inst);
    std::printf("Lin-Kernighan (iterated, %.1fs budget): length=%ld  (%.2fs)\n", timeBudget, bestLen, since(t2));

    if (!validateTour(best)) {
        std::fprintf(stderr, "ERROR: resulting tour is not a valid permutation!\n");
        return 2;
    }
    std::printf("Tour validated OK.\n");

    if (knownOptimal) {
        double gapPct = 100.0 * (static_cast<double>(bestLen) - static_cast<double>(*knownOptimal)) /
                        static_cast<double>(*knownOptimal);
        std::printf("Known optimal: %ld  gap: %.3f%%\n", *knownOptimal, gapPct);
    }

    std::printf("Total time: %.2fs\n", since(t0));
    return 0;
}
