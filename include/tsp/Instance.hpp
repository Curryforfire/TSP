#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace tsp {

enum class EdgeWeightType { EUC_2D, CEIL_2D, ATT, GEO };

struct Point {
    double x = 0.0;
    double y = 0.0;
};

// Parses a TSPLIB .tsp file (NODE_COORD_SECTION based instances) and
// provides integer edge-weight evaluation per the TSPLIB spec.
class Instance {
public:
    static Instance readFile(const std::string& path);

    int size() const { return static_cast<int>(points_.size()); }
    const std::string& name() const { return name_; }
    EdgeWeightType edgeWeightType() const { return type_; }
    const Point& point(int city) const { return points_[city]; }

    long dist(int a, int b) const;

private:
    std::string name_;
    EdgeWeightType type_ = EdgeWeightType::EUC_2D;
    std::vector<Point> points_;
};

}  // namespace tsp
