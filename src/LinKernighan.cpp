#include "tsp/LinKernighan.hpp"

#include <algorithm>
#include <chrono>
#include <cstdio>

namespace tsp {

LinKernighan::LinKernighan(const Instance& inst, const NeighborLists& nl, int maxDepth, int breadth)
    : inst_(inst), nl_(nl), maxDepth_(maxDepth), breadth_(breadth), inQueue_(inst.size(), 0) {}

void LinKernighan::activate(int city) {
    if (!inQueue_[city]) {
        inQueue_[city] = 1;
        queue_.push_back(city);
    }
}

bool LinKernighan::lkStep(Tour& tour, int t1, int tLast, long gain, int depth, std::vector<int>& chain,
                           std::vector<char>& inChain) {
    // Re-derive direction from the live tour rather than trusting a flag
    // carried down the recursion: when the shorter-arc optimization in
    // reverseSegment picks the complementary side, and that side happens
    // to contain t1, applying it silently swaps whether tLast sits on
    // t1's succ side or pred side. Any stale "forward" assumption from an
    // earlier level would then apply the wrong t4 formula and corrupt the
    // tour, so this must be checked fresh every time.
    bool forward = tour.succ(t1) == tLast;

    const auto& cands = nl_.neighbors(tLast);
    int tried = 0;
    int tLastSucc = tour.succ(tLast);
    int tLastPred = tour.pred(tLast);

    for (int t3 : cands) {
        if (tried >= breadth_) break;
        long dLastT3 = inst_.dist(tLast, t3);
        if (dLastT3 >= gain) break;  // sorted ascending: no candidate beyond this can give positive g1
        if (t3 == t1 || t3 == tLastSucc || t3 == tLastPred) continue;
        if (inChain[t3]) continue;

        int t4 = forward ? tour.pred(t3) : tour.succ(t3);
        if (t4 == t1 || inChain[t4]) continue;

        ++tried;
        long g1 = gain - dLastT3;
        long newGain = g1 + inst_.dist(t3, t4);

        // Applying this reversal makes (tLast,t3) and (t1,t4) tour edges;
        // save the exact physical range touched so a failed branch can be
        // undone precisely (reversing the same range twice is an exact
        // involution -- re-deriving positions from city labels after the
        // fact is not, since the shorter-side optimization can pick a
        // different physical arc once positions have shifted).
        Tour::ReverseHandle h = forward ? tour.reverseSegment(tLast, t4) : tour.reverseSegment(t4, tLast);

        long closeGain = newGain - inst_.dist(t4, t1);
        if (closeGain > 0) {
            chain.push_back(t3);
            chain.push_back(t4);
            return true;
        }

        if (depth < maxDepth_) {
            chain.push_back(t3);
            chain.push_back(t4);
            inChain[t3] = inChain[t4] = 1;
            if (lkStep(tour, t1, t4, newGain, depth + 1, chain, inChain)) {
                return true;
            }
            inChain[t3] = inChain[t4] = 0;
            chain.pop_back();
            chain.pop_back();
        }

        tour.reverseRange(h);  // undo this level's reversal, try the next candidate
    }
    return false;
}

bool LinKernighan::improveCity(Tour& tour, int t1) {
    int n = tour.size();
    std::vector<char> inChain(n, 0);
    for (bool forward : {true, false}) {
        int t2 = forward ? tour.succ(t1) : tour.pred(t1);
        long gain0 = inst_.dist(t1, t2);
        std::vector<int> chain{t1, t2};
        inChain.assign(n, 0);
        inChain[t1] = inChain[t2] = 1;
        if (lkStep(tour, t1, t2, gain0, 1, chain, inChain)) {
            for (int c : chain) activate(c);
            return true;
        }
    }
    return false;
}

bool LinKernighan::orOptCity(Tour& tour, int c) {
    for (int segLen = 1; segLen <= 3; ++segLen) {
        int segStart = c;
        int segEnd = c;
        std::vector<int> seg{segStart};
        for (int i = 1; i < segLen; ++i) {
            segEnd = tour.succ(segEnd);
            seg.push_back(segEnd);
        }
        int p = tour.pred(segStart);
        int q = tour.succ(segEnd);
        if (p == segEnd || q == segStart) break;  // segment covers (almost) whole tour

        long removed = inst_.dist(p, segStart) + inst_.dist(segEnd, q);
        long bridge = inst_.dist(p, q);
        long gainRemove = removed - bridge;
        if (gainRemove <= 0) continue;

        std::vector<char> inSeg(tour.size(), 0);
        for (int s : seg) inSeg[s] = 1;

        for (int endpoint : {segStart, segEnd}) {
            for (int cand : nl_.neighbors(endpoint)) {
                if (inSeg[cand]) continue;
                for (bool useAsA : {true, false}) {
                    int a = useAsA ? cand : tour.pred(cand);
                    int b = useAsA ? tour.succ(cand) : cand;
                    if (inSeg[a] || inSeg[b]) continue;
                    if (a == p && b == q) continue;  // same location, no-op

                    long dab = inst_.dist(a, b);
                    long addedFwd = inst_.dist(a, segStart) + inst_.dist(segEnd, b);
                    long addedRev = inst_.dist(a, segEnd) + inst_.dist(segStart, b);
                    bool rev = addedRev < addedFwd;
                    long added = rev ? addedRev : addedFwd;
                    long gain = gainRemove + dab - added;

                    if (gain > 0) {
                        std::vector<int> ord = tour.order();
                        std::vector<int> rest;
                        rest.reserve(ord.size() - seg.size());
                        for (int city : ord) {
                            if (!inSeg[city]) rest.push_back(city);
                        }
                        auto it = std::find(rest.begin(), rest.end(), a);
                        int idx = static_cast<int>(it - rest.begin());
                        std::vector<int> segToInsert = seg;
                        if (rev) std::reverse(segToInsert.begin(), segToInsert.end());

                        std::vector<int> newOrder;
                        newOrder.reserve(ord.size());
                        newOrder.insert(newOrder.end(), rest.begin(), rest.begin() + idx + 1);
                        newOrder.insert(newOrder.end(), segToInsert.begin(), segToInsert.end());
                        newOrder.insert(newOrder.end(), rest.begin() + idx + 1, rest.end());
                        tour.rebuild(newOrder);

                        activate(p);
                        activate(q);
                        activate(a);
                        activate(b);
                        activate(segStart);
                        activate(segEnd);
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

void LinKernighan::runToLocalOptimum(Tour& tour) {
    int n = tour.size();
    std::fill(inQueue_.begin(), inQueue_.end(), 0);
    queue_.clear();
    for (int c = 0; c < n; ++c) activate(c);

    while (!queue_.empty()) {
        int t1 = queue_.front();
        queue_.pop_front();
        inQueue_[t1] = 0;

        if (improveCity(tour, t1) || orOptCity(tour, t1)) {
            activate(t1);
        }
    }
}

void LinKernighan::optimize(Tour& tour) { runToLocalOptimum(tour); }

void LinKernighan::doubleBridge(Tour& tour, std::mt19937& rng) {
    int n = tour.size();
    if (n < 8) return;
    std::uniform_int_distribution<int> dist(1, n - 1);
    std::vector<int> cuts = {dist(rng), dist(rng), dist(rng)};
    std::sort(cuts.begin(), cuts.end());
    if (cuts[0] == cuts[1] || cuts[1] == cuts[2]) return;

    const auto& ord = tour.order();
    std::vector<int> newOrder;
    newOrder.reserve(n);
    newOrder.insert(newOrder.end(), ord.begin(), ord.begin() + cuts[0]);
    newOrder.insert(newOrder.end(), ord.begin() + cuts[2], ord.end());
    newOrder.insert(newOrder.end(), ord.begin() + cuts[1], ord.begin() + cuts[2]);
    newOrder.insert(newOrder.end(), ord.begin() + cuts[0], ord.begin() + cuts[1]);
    // Segments recombined as A-D-C-B, the classic non-sequential
    // double-bridge 4-opt move that 2-opt/LK cannot trivially undo.

    tour.rebuild(newOrder);
    for (int c : newOrder) activate(c);
}

Tour LinKernighan::run(Tour initial, double timeLimitSeconds, unsigned seed, bool verbose) {
    using clock = std::chrono::steady_clock;
    auto start = clock::now();
    auto elapsed = [&] { return std::chrono::duration<double>(clock::now() - start).count(); };

    std::mt19937 rng(seed);

    optimize(initial);
    Tour best = initial;
    long bestLen = best.length(inst_);
    if (verbose) {
        std::fprintf(stderr, "[lk] initial local optimum: %ld (t=%.2fs)\n", bestLen, elapsed());
    }

    Tour current = best;
    long currentLen = bestLen;
    long iter = 0;

    while (elapsed() < timeLimitSeconds) {
        ++iter;
        doubleBridge(current, rng);
        runToLocalOptimum(current);
        currentLen = current.length(inst_);

        if (currentLen < bestLen) {
            best = current;
            bestLen = currentLen;
            if (verbose) {
                std::fprintf(stderr, "[lk] iter %ld: improved to %ld (t=%.2fs)\n", iter, bestLen, elapsed());
            }
        } else {
            current = best;
            currentLen = bestLen;
        }
    }

    if (verbose) {
        std::fprintf(stderr, "[lk] done: %ld after %ld perturbations (t=%.2fs)\n", bestLen, iter, elapsed());
    }
    return best;
}

}  // namespace tsp
