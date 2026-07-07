#include "tsp/Instance.hpp"

#include <cmath>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace tsp {

namespace {

EdgeWeightType parseEdgeWeightType(const std::string& token) {
    if (token == "EUC_2D") return EdgeWeightType::EUC_2D;
    if (token == "CEIL_2D") return EdgeWeightType::CEIL_2D;
    if (token == "ATT") return EdgeWeightType::ATT;
    if (token == "GEO") return EdgeWeightType::GEO;
    throw std::runtime_error("Unsupported EDGE_WEIGHT_TYPE: " + token);
}

std::string trim(const std::string& s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == std::string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

}  // namespace

Instance Instance::readFile(const std::string& path) {
    std::ifstream in(path);
    if (!in) throw std::runtime_error("Cannot open TSPLIB file: " + path);

    Instance inst;
    int dimension = 0;
    std::string line;

    while (std::getline(in, line)) {
        std::string content = trim(line);
        if (content.empty()) continue;

        if (content == "NODE_COORD_SECTION") {
            inst.points_.resize(dimension);
            for (int i = 0; i < dimension; ++i) {
                int id;
                double x, y;
                if (!(in >> id >> x >> y)) {
                    throw std::runtime_error("Malformed NODE_COORD_SECTION in " + path);
                }
                if (id < 1 || id > dimension) {
                    throw std::runtime_error("Node id out of range in " + path);
                }
                inst.points_[id - 1] = Point{x, y};
            }
            continue;
        }
        if (content == "EOF") break;

        size_t colon = content.find(':');
        if (colon == std::string::npos) continue;
        std::string key = trim(content.substr(0, colon));
        std::string value = trim(content.substr(colon + 1));

        if (key == "NAME") {
            inst.name_ = value;
        } else if (key == "DIMENSION") {
            dimension = std::stoi(value);
        } else if (key == "EDGE_WEIGHT_TYPE") {
            inst.type_ = parseEdgeWeightType(value);
        }
    }

    if (inst.points_.empty()) {
        throw std::runtime_error("No coordinates parsed from " + path);
    }
    return inst;
}

long Instance::dist(int a, int b) const {
    const Point& pa = points_[a];
    const Point& pb = points_[b];
    double dx = pa.x - pb.x;
    double dy = pa.y - pb.y;

    switch (type_) {
        case EdgeWeightType::EUC_2D:
            return std::lround(std::sqrt(dx * dx + dy * dy));
        case EdgeWeightType::CEIL_2D:
            return static_cast<long>(std::ceil(std::sqrt(dx * dx + dy * dy)));
        case EdgeWeightType::ATT: {
            double rij = std::sqrt((dx * dx + dy * dy) / 10.0);
            long tij = std::lround(rij);
            return tij < rij ? tij + 1 : tij;
        }
        case EdgeWeightType::GEO: {
            // Not used by att532 but kept for completeness.
            constexpr double PI = 3.141592653589793;
            auto toRad = [](double coord) {
                double deg = std::floor(coord);
                double min = coord - deg;
                return PI * (deg + 5.0 * min / 3.0) / 180.0;
            };
            double lat1 = toRad(pa.x), lon1 = toRad(pa.y);
            double lat2 = toRad(pb.x), lon2 = toRad(pb.y);
            constexpr double RRR = 6378.388;
            double q1 = std::cos(lon1 - lon2);
            double q2 = std::cos(lat1 - lat2);
            double q3 = std::cos(lat1 + lat2);
            return static_cast<long>(RRR * std::acos(0.5 * ((1.0 + q1) * q2 - (1.0 - q1) * q3)) + 1.0);
        }
    }
    throw std::runtime_error("Unhandled edge weight type");
}

}  // namespace tsp
