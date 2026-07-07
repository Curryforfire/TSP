#include "tsp/Construct.hpp"

#include <algorithm>
#include <array>
#include <numeric>
#include <vector>

namespace tsp {

namespace {

struct UnionFind {
    std::vector<int> parent, rank_;
    explicit UnionFind(int n) : parent(n), rank_(n, 0) { std::iota(parent.begin(), parent.end(), 0); }
    int find(int x) {
        while (parent[x] != x) {
            parent[x] = parent[parent[x]];
            x = parent[x];
        }
        return x;
    }
    void unite(int a, int b) {
        a = find(a);
        b = find(b);
        if (a == b) return;
        if (rank_[a] < rank_[b]) std::swap(a, b);
        parent[b] = a;
        if (rank_[a] == rank_[b]) rank_[a]++;
    }
};

}  // namespace

Tour greedyEdgeConstruct(const Instance& inst, const NeighborLists& nl) {
    int n = inst.size();

    std::vector<std::pair<long, std::pair<int, int>>> candidates;
    candidates.reserve(static_cast<size_t>(n) * 8);
    for (int c = 0; c < n; ++c) {
        for (int other : nl.neighbors(c)) {
            if (other > c) candidates.emplace_back(inst.dist(c, other), std::make_pair(c, other));
        }
    }
    std::sort(candidates.begin(), candidates.end());

    std::vector<std::array<int, 2>> adj(n, {-1, -1});
    std::vector<int> degree(n, 0);
    UnionFind uf(n);
    int edgesAdded = 0;

    for (auto& [w, uv] : candidates) {
        if (edgesAdded == n - 1) break;
        auto [u, v] = uv;
        if (degree[u] >= 2 || degree[v] >= 2) continue;
        if (uf.find(u) == uf.find(v)) continue;
        adj[u][degree[u]++] = v;
        adj[v][degree[v]++] = u;
        uf.unite(u, v);
        ++edgesAdded;
    }

    // Build path fragments from the partial matching above.
    std::vector<char> visited(n, 0);
    std::vector<std::vector<int>> fragments;
    for (int c = 0; c < n; ++c) {
        if (visited[c] || degree[c] == 2) continue;
        std::vector<int> path;
        int prev = -1, cur = c;
        while (cur != -1 && !visited[cur]) {
            visited[cur] = 1;
            path.push_back(cur);
            int nxt = (adj[cur][0] != prev) ? adj[cur][0] : adj[cur][1];
            prev = cur;
            cur = nxt;
        }
        fragments.push_back(std::move(path));
    }
    // Any left-over vertices form isolated 2-cycles' remnants (shouldn't
    // normally happen given the endpoint scan above, but guard anyway).
    for (int c = 0; c < n; ++c) {
        if (!visited[c]) {
            visited[c] = 1;
            fragments.push_back({c});
        }
    }

    // Stitch fragments into a single tour via nearest-endpoint merging.
    std::vector<int> merged = std::move(fragments.front());
    fragments.erase(fragments.begin());
    while (!fragments.empty()) {
        int tail = merged.back();
        size_t bestIdx = 0;
        bool bestReversed = false;
        long bestDist = -1;
        for (size_t i = 0; i < fragments.size(); ++i) {
            long dFront = inst.dist(tail, fragments[i].front());
            long dBack = inst.dist(tail, fragments[i].back());
            if (bestDist < 0 || dFront < bestDist) {
                bestDist = dFront;
                bestIdx = i;
                bestReversed = false;
            }
            if (dBack < bestDist) {
                bestDist = dBack;
                bestIdx = i;
                bestReversed = true;
            }
        }
        std::vector<int> frag = std::move(fragments[bestIdx]);
        fragments.erase(fragments.begin() + static_cast<long>(bestIdx));
        if (bestReversed) std::reverse(frag.begin(), frag.end());
        merged.insert(merged.end(), frag.begin(), frag.end());
    }

    return Tour(merged);
}

}  // namespace tsp
