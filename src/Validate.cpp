#include "Validate.hpp"
#include <glm/glm.hpp>

using namespace deepnest;
using namespace std;

std::vector<string> validate(const std::vector<Polygon> &origPolys, const vector<Polygon> &placed);
vector<string> validate() {
    vector<string> errors;

    if (origPolys.size() != placed.size()) {
        errors.push_back("validate() error: origPolys and placed vectors have different sizes");
        return errors;
    }

    // 1. Geometry preservation: placement must be a rigid-body transform.
    //    Every edge of the placed polygon must have the same length as the
    //    corresponding edge of the original polygon.
    for (size_t i = 0; i < origPolys.size(); i++) {
        if (origPolys[i].empty())
            continue;
        if (origPolys[i].size() != placed[i].size()) {
            errors.push_back(
                "Part " + to_string(i) + ": vertex count changed after placement (was " +
                to_string(origPolys[i].size()) + ", now " + to_string(placed[i].size()) + ")");
            continue;
        }
        int n = (int)origPolys[i].size();
        for (int k = 0; k < n; k++) {
            double origLen = glm::length(origPolys[i][(k + 1) % n] - origPolys[i][k]);
            double placedLen = glm::length(placed[i][(k + 1) % n] - placed[i][k]);
            if (abs(origLen - placedLen) > 1e-6) {
                errors.push_back("Part " + to_string(i) + ": edge " + to_string(k) +
                                 " length changed after placement (rigid-body violation)");
            }
        }
    }

    // 2. No two sheet-level parts may overlap, and all pairs must respect kerf.
    //    Use a small epsilon on the kerf check to tolerate floating-point noise.
    const double kerfEps = 1e-4; // 1 µm — well below any practical kerf value
    for (size_t i = 0; i < origPolys.size(); i++) {
        if (placements[i].hostPartIndex != -1 || placed[i].empty())
            continue;
        for (size_t j = i + 1; j < parts.size(); j++) {
            if (placements[j].hostPartIndex != -1 || placed[j].empty())
                continue;
            if (polygonsOverlap(placed[i], placed[j])) {
                errors.push_back("Parts " + to_string(i) + " and " + to_string(j) +
                                 " overlap on the sheet");
            } else if (kerf_cm > 0.0) {
                double dist = polygonMinDistance(placed[i], placed[j]);
                if (dist < kerf_cm - kerfEps) {
                    errors.push_back("Parts " + to_string(i) + " and " + to_string(j) +
                                     " are closer than the kerf (" + to_string(dist) + " cm < " +
                                     to_string(kerf_cm) + " cm)");
                }
            }
        }
    }

    // 3. Every hole-placed part must lie fully inside its host hole and must not
    //    overlap other parts sharing the same hole.
    for (size_t i = 0; i < parts.size(); i++) {
        if (placements[i].hostPartIndex == -1 || placed[i].empty())
            continue;
        int hostIdx = placements[i].hostPartIndex;
        int holeIdx = placements[i].hostHoleIndex;

        if (hostIdx < 0 || hostIdx >= (int)parts.size()) {
            errors.push_back("Part " + to_string(i) + ": host part index " + to_string(hostIdx) +
                             " is out of range");
            continue;
        }
        auto holeParts = parts[hostIdx]->toHolePolygons();
        if (holeIdx < 0 || holeIdx >= (int)holeParts.size() || !holeParts[holeIdx] ||
            holeParts[holeIdx]->empty()) {
            errors.push_back("Part " + to_string(i) + ": hole index " + to_string(holeIdx) +
                             " is invalid for host part " + to_string(hostIdx));
            continue;
        }

        transformer_t hostT = computePlacementTransformer(origPolys[hostIdx], placements[hostIdx]);
        polygon_t placedHole = transformPolygon(*holeParts[holeIdx], hostT);

        bool outsideHole = false;
        for (const point_t &p : placed[i]) {
            if (!pointInPolygon(p, placedHole)) {
                outsideHole = true;
                break;
            }
        }
        if (outsideHole) {
            errors.push_back("Part " + to_string(i) + " extends outside hole " +
                             to_string(holeIdx) + " of part " + to_string(hostIdx));
        }

        for (size_t j = i + 1; j < parts.size(); j++) {
            if (placements[j].hostPartIndex != hostIdx || placements[j].hostHoleIndex != holeIdx ||
                placed[j].empty())
                continue;
            if (polygonsOverlap(placed[i], placed[j])) {
                errors.push_back("Parts " + to_string(i) + " and " + to_string(j) +
                                 " overlap inside hole " + to_string(holeIdx) + " of part " +
                                 to_string(hostIdx));
            }
        }
    }

    return errors;
}
