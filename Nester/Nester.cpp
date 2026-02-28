#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>
#include <random>

#include "Nester.hpp"

namespace nester {

// from https://stackoverflow.com/questions/11826554/standard-no-op-output-stream
class NullBuffer : public std::streambuf {
  public:
    int overflow(int c) { return c; }
};

class NullStream : public std::ostream {
  public:
    NullStream() : std::ostream(&m_sb) {}

  private:
    NullBuffer m_sb;
};

transformer_t makeTransformation(double angle, double x, double y) {
    transformer_t mat(1.0); // Identity matrix
    mat = glm::translate(mat, glm::dvec2(x, y));
    // Negate angle: GLM ≥ 1.0.0 uses CCW-positive convention (standard math),
    // but the rest of this codebase was written with the pre-1.0.0 convention
    // where negative angle = CCW.  Negating restores the original semantics.
    mat = glm::rotate(mat, glm::radians(-angle));
    return mat;
}

void NesterLine::setStartPoint(point_t p) { start = p; }
void NesterLine::setEndPoint(point_t p) { end = p; }
point_t NesterLine::getStartPoint() const { return start; }

void NesterLine::write(shared_ptr<FileWriter> writer, color_t color,
                       transformer_t &transformer) const {
    glm::dvec3 tStart3 = transformer * glm::dvec3(start, 1.0);
    glm::dvec3 tEnd3 = transformer * glm::dvec3(end, 1.0);

    point_t tStart(tStart3.x, tStart3.y);
    point_t tEnd(tEnd3.x, tEnd3.y);

    writer->line(tStart, tEnd, color);
}

BoundingBox NesterLine::getBoundingBox() const {
    BoundingBox bb;
    bb.minX = min(start.x, end.x);
    bb.minY = min(start.y, end.y);
    bb.maxX = max(start.x, end.x);
    bb.maxY = max(start.y, end.y);

    return bb;
}

void NesterNurbs::addControlPoint(double x, double y) { controlPoints.push_back(point_t(x, y)); }

void NesterNurbs::addKnots(vector<double> ks) { knots.insert(knots.end(), ks.begin(), ks.end()); };

void NesterNurbs::write(shared_ptr<FileWriter> writer, color_t color,
                        transformer_t &transformer) const {
    // Not implemented
}

BoundingBox NesterNurbs::getBoundingBox() const {
    BoundingBox bb;
    // Not implemented
    return bb;
}

void NesterLoop::addEdge(NesterEdge_p edge) { edges.push_back(edge); }

void NesterLoop::write(shared_ptr<FileWriter> writer, color_t color,
                       transformer_t &transformer) const {
    for (NesterEdge_p edge : edges) {
        edge->write(writer, color, transformer);
    }
}

BoundingBox NesterLoop::getBoundingBox() const {
    BoundingBox bb;
    for (NesterEdge_p edge : edges) {
        bb.join(edge->getBoundingBox());
    }

    return bb;
}

polygon_p NesterLoop::toPolygon() const {
    auto poly = make_shared<polygon_t>();
    for (const NesterEdge_p &e : edges) {
        if (const NesterLine *line = dynamic_cast<const NesterLine *>(e.get()))
            poly->push_back(line->getStartPoint());
    }
    return poly;
}

void NesterPart::setOuterRing(NesterRing_p ring) { outer_ring = ring; }
void NesterPart::addInnerRing(NesterRing_p ring) { inner_rings.push_back(ring); }

void NesterPart::write(shared_ptr<FileWriter> writer, transformer_t &transformer,
                       color_t outerColor, color_t innerColor) const {
    outer_ring->write(writer, outerColor, transformer);
    for (NesterRing_p r : inner_rings) {
        r->write(writer, innerColor, transformer);
    }
}

BoundingBox NesterPart::getBoundingBox() const {
    BoundingBox bb;
    bb.join(outer_ring->getBoundingBox());
    for (NesterRing_p r : inner_rings) {
        bb.join(r->getBoundingBox());
    }

    return bb;
}

polygon_p NesterPart::toPolygon() const { return outer_ring->toPolygon(); }

vector<polygon_p> NesterPart::toHolePolygons() const {
    vector<polygon_p> result;
    for (const NesterRing_p &r : inner_rings)
        result.push_back(r->toPolygon());
    return result;
}

// -------------------------------------------------------------------------
// Geometry helpers
// -------------------------------------------------------------------------

polygon_t transformPolygon(const polygon_t &poly, transformer_t t) {
    polygon_t result;
    result.reserve(poly.size());
    for (const point_t &p : poly) {
        glm::dvec3 p3(p.x, p.y, 1.0);
        glm::dvec3 r3 = t * p3;
        result.push_back(point_t(r3.x, r3.y));
    }
    return result;
}

BoundingBox computeBB(const polygon_t &poly) {
    BoundingBox bb;
    for (const point_t &p : poly) {
        if (p.x < bb.minX)
            bb.minX = p.x;
        if (p.x > bb.maxX)
            bb.maxX = p.x;
        if (p.y < bb.minY)
            bb.minY = p.y;
        if (p.y > bb.maxY)
            bb.maxY = p.y;
    }
    return bb;
}

bool segmentsIntersect(point_t a1, point_t a2, point_t b1, point_t b2) {
    // Cross-product orientation test
    auto cross = [](point_t o, point_t a, point_t b) {
        return (a.x - o.x) * (b.y - o.y) - (a.y - o.y) * (b.x - o.x);
    };
    double d1 = cross(b1, b2, a1);
    double d2 = cross(b1, b2, a2);
    double d3 = cross(a1, a2, b1);
    double d4 = cross(a1, a2, b2);

    if (((d1 > 0.0 && d2 < 0.0) || (d1 < 0.0 && d2 > 0.0)) &&
        ((d3 > 0.0 && d4 < 0.0) || (d3 < 0.0 && d4 > 0.0)))
        return true;

    auto onSeg = [](point_t p, point_t a, point_t b) {
        return p.x >= min(a.x, b.x) && p.x <= max(a.x, b.x) && p.y >= min(a.y, b.y) &&
               p.y <= max(a.y, b.y);
    };
    if (d1 == 0.0 && onSeg(a1, b1, b2))
        return true;
    if (d2 == 0.0 && onSeg(a2, b1, b2))
        return true;
    if (d3 == 0.0 && onSeg(b1, a1, a2))
        return true;
    if (d4 == 0.0 && onSeg(b2, a1, a2))
        return true;

    return false;
}

bool pointInPolygon(point_t p, const polygon_t &poly) {
    if (poly.empty())
        return false;
    int n = (int)poly.size();
    bool inside = false;
    for (int i = 0, j = n - 1; i < n; j = i++) {
        const point_t &pi = poly[i];
        const point_t &pj = poly[j];
        if (((pi.y > p.y) != (pj.y > p.y)) &&
            (p.x < (pj.x - pi.x) * (p.y - pi.y) / (pj.y - pi.y) + pi.x))
            inside = !inside;
    }
    return inside;
}

bool polygonsOverlap(const polygon_t &a, const polygon_t &b) {
    if (a.empty() || b.empty())
        return false;

    // Fast bounding-box reject
    BoundingBox ba = computeBB(a);
    BoundingBox bb_b = computeBB(b);
    if (ba.maxX < bb_b.minX || bb_b.maxX < ba.minX || ba.maxY < bb_b.minY || bb_b.maxY < ba.minY)
        return false;

    // Edge intersection test
    int na = (int)a.size(), nb = (int)b.size();
    for (int i = 0; i < na; i++) {
        point_t a1 = a[i], a2 = a[(i + 1) % na];
        for (int j = 0; j < nb; j++) {
            if (segmentsIntersect(a1, a2, b[j], b[(j + 1) % nb]))
                return true;
        }
    }

    // Containment: one polygon fully inside the other
    if (pointInPolygon(a[0], b))
        return true;
    if (pointInPolygon(b[0], a))
        return true;

    return false;
}

static double ptSegDist(point_t p, point_t a, point_t b) {
    point_t ab = b - a;
    point_t ap = p - a;
    double len2 = glm::dot(ab, ab);
    if (len2 == 0.0)
        return glm::length(p - a);
    double t = glm::dot(ap, ab) / len2;
    if (t < 0.0)
        t = 0.0;
    if (t > 1.0)
        t = 1.0;
    point_t closest = a + t * ab;
    return glm::length(p - closest);
}

double polygonMinDistance(const polygon_t &a, const polygon_t &b) {
    if (a.empty() || b.empty())
        return numeric_limits<double>::infinity();
    double minDist = numeric_limits<double>::infinity();
    int na = (int)a.size(), nb = (int)b.size();
    for (int i = 0; i < na; i++) {
        point_t a1 = a[i], a2 = a[(i + 1) % na];
        for (int j = 0; j < nb; j++) {
            point_t b1 = b[j], b2 = b[(j + 1) % nb];
            double d;
            d = ptSegDist(a1, b1, b2);
            if (d < minDist)
                minDist = d;
            d = ptSegDist(a2, b1, b2);
            if (d < minDist)
                minDist = d;
            d = ptSegDist(b1, a1, a2);
            if (d < minDist)
                minDist = d;
            d = ptSegDist(b2, a1, a2);
            if (d < minDist)
                minDist = d;
        }
    }
    return minDist;
}

// -------------------------------------------------------------------------
// SA internal helpers (static — not exposed outside this translation unit)
// -------------------------------------------------------------------------

// Build the transformer that places a part's outer polygon so its rotated
// bounding-box lower-left corner is at (pl.x, pl.y).
static transformer_t computePlacementTransformer(const polygon_t &outerPoly, const Placement &pl) {
    if (outerPoly.empty())
        return transformer_t(1.0);
    polygon_t rotated = transformPolygon(outerPoly, makeTransformation(pl.angle, 0.0, 0.0));
    BoundingBox rbb = computeBB(rotated);
    return makeTransformation(pl.angle, pl.x - (double)rbb.minX, pl.y - (double)rbb.minY);
}

// Apply a placement to a polygon and return the placed polygon.
polygon_t computePlacedPolygon(const polygon_t &poly, const Placement &pl) {
    if (poly.empty())
        return {};
    transformer_t t = computePlacementTransformer(poly, pl);
    return transformPolygon(poly, t);
}

struct HoleSlot {
    int partIndex;
    int holeIndex;
};

// For each part pi, list all (partIndex, holeIndex) pairs whose bounding box
// is large enough to potentially contain pi.
static vector<vector<HoleSlot>> buildHoleCandidates(const vector<NesterPart_p> &parts,
                                                    const vector<polygon_t> &partPolys) {
    vector<vector<HoleSlot>> candidates(parts.size());
    for (size_t qi = 0; qi < parts.size(); qi++) {
        auto holeParts = parts[qi]->toHolePolygons();
        for (size_t hi = 0; hi < holeParts.size(); hi++) {
            if (!holeParts[hi] || holeParts[hi]->empty())
                continue;
            BoundingBox holeBB = computeBB(*holeParts[hi]);
            double holeMinDim = min((double)holeBB.width(), (double)holeBB.height());
            for (size_t pi = 0; pi < parts.size(); pi++) {
                if (pi == qi)
                    continue;
                if (partPolys[pi].empty())
                    continue;
                BoundingBox partBB = computeBB(partPolys[pi]);
                double partMinDim = min((double)partBB.width(), (double)partBB.height());
                if (partMinDim < holeMinDim) {
                    candidates[pi].push_back({(int)qi, (int)hi});
                }
            }
        }
    }
    return candidates;
}

// Place parts in a single horizontal row (used as starting state for SA).
static vector<Placement> makeInitialPlacement(const vector<NesterPart_p> &parts,
                                              const vector<polygon_t> &origPolys, double kerf_cm) {
    vector<Placement> placements;
    double offset = 0.0;
    for (size_t i = 0; i < parts.size(); i++) {
        Placement pl;
        pl.hostPartIndex = -1;
        pl.hostHoleIndex = -1;
        pl.angle = 0.0;
        pl.x = 0.0;
        pl.y = 0.0;

        if (origPolys[i].empty()) {
            placements.push_back(pl);
            continue;
        }

        // Prefer portrait orientation (tall > wide after rotation)
        BoundingBox bb0 = computeBB(origPolys[i]);
        if (bb0.width() > bb0.height()) {
            pl.angle = -90.0;
        }

        polygon_t rotated = transformPolygon(origPolys[i], makeTransformation(pl.angle, 0.0, 0.0));
        BoundingBox rbb = computeBB(rotated);

        pl.x = offset;
        pl.y = 0.0;
        const double gap = (kerf_cm > 0.0) ? kerf_cm : 1e-9;
        offset += gap + (double)rbb.width();
        placements.push_back(pl);
    }
    return placements;
}

// Energy = bounding-box area of sheet-level parts only.
static double computeEnergy(const vector<Placement> &placements,
                            const vector<polygon_t> &cachedPolys) {
    BoundingBox totalBB;
    bool anyPart = false;
    for (size_t i = 0; i < placements.size(); i++) {
        if (placements[i].hostPartIndex != -1)
            continue;
        if (cachedPolys[i].empty())
            continue;
        totalBB.join(computeBB(cachedPolys[i]));
        anyPart = true;
    }
    if (!anyPart)
        return 0.0;
    return (double)(totalBB.width() * totalBB.height());
}

// Check that part idx does not collide with any other part under the current
// placement state.  Only the moved part (idx) is validated — callers ensure
// every other part is already in a valid state.
static bool isPartValid(int idx, const vector<Placement> &placements,
                        const vector<polygon_t> &cachedPolys, const vector<NesterPart_p> &parts,
                        const vector<polygon_t> &origPolys, double kerf_cm) {
    const Placement &pl = placements[idx];
    const polygon_t &poly = cachedPolys[idx];
    if (poly.empty())
        return true;

    if (pl.hostPartIndex == -1) {
        // Sheet-level: check against every other sheet-level part.
        BoundingBox myBB = computeBB(poly);
        for (int j = 0; j < (int)placements.size(); j++) {
            if (j == idx)
                continue;
            if (placements[j].hostPartIndex != -1)
                continue;
            const polygon_t &other = cachedPolys[j];
            if (other.empty())
                continue;

            // Fast reject: expanded bounding boxes don't touch.
            BoundingBox otherBB = computeBB(other);
            if ((double)myBB.maxX + kerf_cm < (double)otherBB.minX ||
                (double)otherBB.maxX + kerf_cm < (double)myBB.minX ||
                (double)myBB.maxY + kerf_cm < (double)otherBB.minY ||
                (double)otherBB.maxY + kerf_cm < (double)myBB.minY)
                continue;

            if (polygonsOverlap(poly, other))
                return false;
            if (kerf_cm > 0.0 && polygonMinDistance(poly, other) < kerf_cm)
                return false;
        }
    } else {
        // Hole-placed: must be fully inside the host hole and not overlap
        // other parts in the same hole.
        int hostIdx = pl.hostPartIndex;
        int holeIdx = pl.hostHoleIndex;
        if (hostIdx < 0 || hostIdx >= (int)parts.size())
            return false;

        auto holeParts = parts[hostIdx]->toHolePolygons();
        if (holeIdx < 0 || holeIdx >= (int)holeParts.size())
            return false;
        if (!holeParts[holeIdx] || holeParts[holeIdx]->empty())
            return false;

        // Transform the hole polygon using the host's placement.
        transformer_t hostT = computePlacementTransformer(origPolys[hostIdx], placements[hostIdx]);
        polygon_t placedHole = transformPolygon(*holeParts[holeIdx], hostT);

        // All vertices of the part must lie inside the hole.
        for (const point_t &p : poly) {
            if (!pointInPolygon(p, placedHole))
                return false;
        }

        // No overlap with other parts sharing the same hole.
        for (int j = 0; j < (int)placements.size(); j++) {
            if (j == idx)
                continue;
            if (placements[j].hostPartIndex != hostIdx || placements[j].hostHoleIndex != holeIdx)
                continue;
            const polygon_t &other = cachedPolys[j];
            if (other.empty())
                continue;
            if (polygonsOverlap(poly, other))
                return false;
            if (kerf_cm > 0.0 && polygonMinDistance(poly, other) < kerf_cm)
                return false;
        }
    }
    return true;
}

// -------------------------------------------------------------------------
// Nester methods
// -------------------------------------------------------------------------

Nester::Nester() : kerf_cm(0.0) { log = make_shared<NullStream>(); }

void Nester::addPart(NesterPart_p part) { parts.push_back(part); }

void Nester::setKerf(double k) { kerf_cm = k; }

vector<Placement> Nester::getPlacements() const { return placements; }

vector<Placement> Nester::computeInitialPlacement() const {
    vector<polygon_t> origPolys(parts.size());
    for (size_t i = 0; i < parts.size(); i++) {
        auto p = parts[i]->toPolygon();
        if (p)
            origPolys[i] = *p;
    }
    return makeInitialPlacement(parts, origPolys, kerf_cm);
}

void Nester::run(std::function<bool(int, int)> progress) {
    if (parts.empty())
        return;

    // Extract outer polygons in their original (untransformed) coordinate system.
    vector<polygon_t> origPolys(parts.size());
    for (size_t i = 0; i < parts.size(); i++) {
        auto p = parts[i]->toPolygon();
        if (p)
            origPolys[i] = *p;
    }

    auto holeCandidates = buildHoleCandidates(parts, origPolys);
    placements = makeInitialPlacement(parts, origPolys, kerf_cm);

    // Compute placed polygons for the initial state.
    vector<polygon_t> cachedPolys(parts.size());
    for (size_t i = 0; i < parts.size(); i++) {
        if (!origPolys[i].empty())
            cachedPolys[i] = computePlacedPolygon(origPolys[i], placements[i]);
    }

    // Track each hole-placed part's offset from the centre of its host hole.
    // When a host part moves, this lets us cascade-update all nested parts.
    vector<pair<double, double>> relInHole(parts.size(), {0.0, 0.0});

    // Compute the centre of the placed hole polygon (hostIdx, holeIdx).
    auto getHoleCentre = [&](int hostIdx, int holeIdx) -> pair<double, double> {
        auto holeParts = parts[hostIdx]->toHolePolygons();
        if (holeIdx < 0 || holeIdx >= (int)holeParts.size() || !holeParts[holeIdx] ||
            holeParts[holeIdx]->empty())
            return {0.0, 0.0};
        transformer_t hostT = computePlacementTransformer(origPolys[hostIdx], placements[hostIdx]);
        polygon_t placedHole = transformPolygon(*holeParts[holeIdx], hostT);
        BoundingBox holeBB = computeBB(placedHole);
        return {(double)(holeBB.minX + holeBB.maxX) * 0.5,
                (double)(holeBB.minY + holeBB.maxY) * 0.5};
    };

    // Update relInHole[i] to reflect part i's current placement vs. hole centre.
    auto syncRelInHole = [&](int i) {
        if (placements[i].hostPartIndex == -1) {
            relInHole[i] = {0.0, 0.0};
            return;
        }
        pair<double, double> c =
            getHoleCentre(placements[i].hostPartIndex, placements[i].hostHoleIndex);
        relInHole[i] = {placements[i].x - c.first, placements[i].y - c.second};
    };

    // After part hostIdx has moved, recursively update all parts nested within it
    // so they preserve their relative offsets from their hole centre.
    std::function<void(int)> cascadeUpdate = [&](int hostIdx) {
        for (int j = 0; j < (int)parts.size(); j++) {
            if (placements[j].hostPartIndex != hostIdx)
                continue;
            pair<double, double> c = getHoleCentre(hostIdx, placements[j].hostHoleIndex);
            placements[j].x = c.first + relInHole[j].first;
            placements[j].y = c.second + relInHole[j].second;
            if (!origPolys[j].empty())
                cachedPolys[j] = computePlacedPolygon(origPolys[j], placements[j]);
            cascadeUpdate(j);
        }
    };

    // --- Greedy pre-pass ---
    // Assign each part to the tightest hole that fits it, processing parts
    // from largest to smallest so every host's placement is known before its
    // tenants are considered.  kerf=0 here — SA enforces gaps later.
    {
        vector<size_t> order(parts.size());
        iota(order.begin(), order.end(), 0);
        sort(order.begin(), order.end(), [&](size_t a, size_t b) {
            if (origPolys[a].empty())
                return false;
            if (origPolys[b].empty())
                return true;
            BoundingBox ba = computeBB(origPolys[a]), bb = computeBB(origPolys[b]);
            return min((double)ba.width(), (double)ba.height()) >
                   min((double)bb.width(), (double)bb.height());
        });

        for (size_t idx : order) {
            if (holeCandidates[idx].empty() || origPolys[idx].empty())
                continue;

            // Sort candidates tightest-first (smallest hole that could hold this part).
            vector<HoleSlot> cands = holeCandidates[idx];
            sort(cands.begin(), cands.end(), [&](const HoleSlot &a, const HoleSlot &b) {
                auto holeMinDim = [&](const HoleSlot &s) -> double {
                    auto hp = parts[s.partIndex]->toHolePolygons();
                    if (s.holeIndex >= (int)hp.size() || !hp[s.holeIndex] ||
                        hp[s.holeIndex]->empty())
                        return 0.0;
                    BoundingBox hbb = computeBB(*hp[s.holeIndex]);
                    return min((double)hbb.width(), (double)hbb.height());
                };
                return holeMinDim(a) < holeMinDim(b);
            });

            for (const HoleSlot &slot : cands) {
                auto holeParts = parts[slot.partIndex]->toHolePolygons();
                if (slot.holeIndex >= (int)holeParts.size() || !holeParts[slot.holeIndex] ||
                    holeParts[slot.holeIndex]->empty())
                    continue;

                transformer_t hostT = computePlacementTransformer(origPolys[slot.partIndex],
                                                                  placements[slot.partIndex]);
                polygon_t placedHole = transformPolygon(*holeParts[slot.holeIndex], hostT);
                BoundingBox holeBB = computeBB(placedHole);

                Placement newPl = placements[idx];
                newPl.hostPartIndex = slot.partIndex;
                newPl.hostHoleIndex = slot.holeIndex;
                polygon_t partRot =
                    transformPolygon(origPolys[idx], makeTransformation(newPl.angle, 0.0, 0.0));
                BoundingBox pbb = computeBB(partRot);
                newPl.x = (double)(holeBB.minX + holeBB.maxX) * 0.5 - (double)pbb.width() * 0.5;
                newPl.y = (double)(holeBB.minY + holeBB.maxY) * 0.5 - (double)pbb.height() * 0.5;

                polygon_t newPoly = computePlacedPolygon(origPolys[idx], newPl);
                Placement oldPl = placements[idx];
                polygon_t oldPoly = cachedPolys[idx];
                placements[idx] = newPl;
                cachedPolys[idx] = newPoly;

                if (isPartValid(idx, placements, cachedPolys, parts, origPolys, 0.0)) {
                    syncRelInHole(idx);
                    break; // keep this placement, move to next part
                }
                placements[idx] = oldPl;
                cachedPolys[idx] = oldPoly;
            }
        }
    }
    // --- End greedy pre-pass ---

    double energy = computeEnergy(placements, cachedPolys);
    if (energy <= 0.0)
        return;

    const double T0 = energy * 0.3;
    const double alpha = 0.995;
    const double T_min = T0 * 1e-4;
    const int MAX_OUTER = 1000;
    const int INNER = max(50, (int)parts.size() * 20);

    double T = T0;

    mt19937 rng(42);
    uniform_real_distribution<double> uniform01(0.0, 1.0);
    uniform_int_distribution<int> partDist(0, (int)parts.size() - 1);

    // Snapshot the greedy state as the best known starting point.
    vector<Placement> bestPlacements = placements;
    vector<polygon_t> bestCachedPolys = cachedPolys;
    double bestEnergy = energy;
    vector<pair<double, double>> bestRelInHole = relInHole;

    for (int outer = 0; outer < MAX_OUTER && T >= T_min; outer++) {
        double stepFrac = sqrt(T / T0);
        double stepSize = stepFrac * 10.0; // max ~10 cm at full temperature
        double rotStep = stepFrac * 180.0; // max 180° rotation

        for (int inner = 0; inner < INNER; inner++) {
            int idx = partDist(rng);
            Placement newPl = placements[idx];

            double r = uniform01(rng);
            if (r < 0.60) {
                // Translate
                normal_distribution<double> nd(0.0, stepSize);
                newPl.x += nd(rng);
                newPl.y += nd(rng);
            } else if (r < 0.85) {
                // Rotate
                normal_distribution<double> nd(0.0, rotStep);
                newPl.angle += nd(rng);
            } else if (!holeCandidates[idx].empty()) {
                // Hole toggle
                if (newPl.hostPartIndex == -1) {
                    // Move into a random candidate hole.
                    uniform_int_distribution<int> holeDist(0, (int)holeCandidates[idx].size() - 1);
                    const HoleSlot &slot = holeCandidates[idx][holeDist(rng)];
                    newPl.hostPartIndex = slot.partIndex;
                    newPl.hostHoleIndex = slot.holeIndex;
                    // Seed position near the centre of the hole.
                    auto holeParts = parts[slot.partIndex]->toHolePolygons();
                    if (!holeParts.empty() && slot.holeIndex < (int)holeParts.size() &&
                        holeParts[slot.holeIndex] && !holeParts[slot.holeIndex]->empty()) {
                        transformer_t hostT = computePlacementTransformer(
                            origPolys[slot.partIndex], placements[slot.partIndex]);
                        polygon_t placedHole = transformPolygon(*holeParts[slot.holeIndex], hostT);
                        BoundingBox holeBB = computeBB(placedHole);
                        // Centre the part inside the hole: pl.x/y is bbox lower-left,
                        // so subtract half the part's rotated dimensions from hole centre.
                        if (!origPolys[idx].empty()) {
                            polygon_t partRotated = transformPolygon(
                                origPolys[idx], makeTransformation(newPl.angle, 0.0, 0.0));
                            BoundingBox partBB = computeBB(partRotated);
                            newPl.x = (double)(holeBB.minX + holeBB.maxX) * 0.5 -
                                      (double)partBB.width() * 0.5;
                            newPl.y = (double)(holeBB.minY + holeBB.maxY) * 0.5 -
                                      (double)partBB.height() * 0.5;
                        }
                    }
                } else {
                    // Return to sheet.
                    newPl.hostPartIndex = -1;
                    newPl.hostHoleIndex = -1;
                }
            } else {
                // Fallback: translate (no hole candidates for this part).
                normal_distribution<double> nd(0.0, stepSize);
                newPl.x += nd(rng);
                newPl.y += nd(rng);
            }

            // Compute the proposed placed polygon.
            polygon_t newPoly;
            if (!origPolys[idx].empty())
                newPoly = computePlacedPolygon(origPolys[idx], newPl);

            // Temporarily apply the proposed move.
            Placement oldPl = placements[idx];
            polygon_t oldPoly = cachedPolys[idx];
            placements[idx] = newPl;
            cachedPolys[idx] = newPoly;

            if (isPartValid(idx, placements, cachedPolys, parts, origPolys, kerf_cm)) {
                // Snapshot state of any parts nested inside idx before cascading,
                // so we can restore them if the cascade produces invalid positions.
                vector<pair<int, pair<Placement, polygon_t>>> cascadeSnapshot;
                {
                    std::function<void(int)> snap = [&](int h) {
                        for (int j = 0; j < (int)parts.size(); j++) {
                            if (placements[j].hostPartIndex != h)
                                continue;
                            cascadeSnapshot.push_back({j, {placements[j], cachedPolys[j]}});
                            snap(j);
                        }
                    };
                    snap(idx);
                }

                // Propagate the host's new position to all nested parts.
                cascadeUpdate(idx);

                // Reject the move if any cascade-updated child is now outside its hole.
                bool cascadeOk = true;
                {
                    std::function<void(int)> checkNested = [&](int h) {
                        for (int j = 0; j < (int)parts.size(); j++) {
                            if (placements[j].hostPartIndex != h)
                                continue;
                            if (!isPartValid(j, placements, cachedPolys, parts, origPolys, kerf_cm))
                                cascadeOk = false;
                            if (cascadeOk)
                                checkNested(j);
                        }
                    };
                    checkNested(idx);
                }

                auto restoreAll = [&]() {
                    placements[idx] = oldPl;
                    cachedPolys[idx] = oldPoly;
                    for (auto &sc : cascadeSnapshot) {
                        placements[sc.first] = sc.second.first;
                        cachedPolys[sc.first] = sc.second.second;
                    }
                };

                if (!cascadeOk) {
                    restoreAll();
                } else {
                    double newEnergy = computeEnergy(placements, cachedPolys);
                    double deltaE = newEnergy - energy;
                    if (deltaE < 0.0 || uniform01(rng) < exp(-deltaE / T)) {
                        // Accept
                        energy = newEnergy;
                        syncRelInHole(idx);
                        if (energy < bestEnergy) {
                            bestEnergy = energy;
                            bestPlacements = placements;
                            bestCachedPolys = cachedPolys;
                            bestRelInHole = relInHole;
                        }
                    } else {
                        restoreAll();
                    }
                }
            } else {
                // Invalid move — reject.
                placements[idx] = oldPl;
                cachedPolys[idx] = oldPoly;
            }
        }
        T *= alpha;
        if (progress) {
            // Temporarily expose bestPlacements so write() works inside the callback,
            // then restore the SA working state so the algorithm can continue correctly.
            auto workingPlacements = placements;
            placements = bestPlacements;
            bool continueRunning = progress(outer + 1, MAX_OUTER);
            placements = workingPlacements;
            if (!continueRunning)
                break;
        }
    }

    // Restore best state found.
    placements = bestPlacements;
    relInHole = bestRelInHole;

    // Normalise: shift sheet-level parts so the combined bounding box
    // starts at (0, 0).
    BoundingBox combinedBB;
    for (size_t i = 0; i < placements.size(); i++) {
        if (placements[i].hostPartIndex != -1)
            continue;
        if (bestCachedPolys[i].empty())
            continue;
        combinedBB.join(computeBB(bestCachedPolys[i]));
    }
    double shiftX = -(double)combinedBB.minX;
    double shiftY = -(double)combinedBB.minY;
    for (Placement &pl : placements) {
        if (pl.hostPartIndex == -1) {
            pl.x += shiftX;
            pl.y += shiftY;
        }
    }

    // Cascade-update nested parts after the normalisation shift.
    // Run passes equal to the nesting depth so multi-level nesting converges.
    for (int pass = 0; pass < (int)parts.size(); pass++) {
        for (size_t j = 0; j < placements.size(); j++) {
            int hostIdx = placements[j].hostPartIndex;
            if (hostIdx == -1)
                continue;
            auto holeParts = parts[hostIdx]->toHolePolygons();
            int holeIdx = placements[j].hostHoleIndex;
            if (holeIdx < 0 || holeIdx >= (int)holeParts.size())
                continue;
            if (!holeParts[holeIdx] || holeParts[holeIdx]->empty())
                continue;
            transformer_t hostT =
                computePlacementTransformer(origPolys[hostIdx], placements[hostIdx]);
            polygon_t placedHole = transformPolygon(*holeParts[holeIdx], hostT);
            BoundingBox holeBB = computeBB(placedHole);
            placements[j].x = (double)(holeBB.minX + holeBB.maxX) * 0.5 + relInHole[j].first;
            placements[j].y = (double)(holeBB.minY + holeBB.maxY) * 0.5 + relInHole[j].second;
        }
    }
}

vector<string> Nester::validate() const {
    vector<string> errors;

    if (placements.size() != parts.size()) {
        errors.push_back("run() has not been called — no placements to validate");
        return errors;
    }

    // Build original and placed polygons once.
    vector<polygon_t> origPolys(parts.size());
    vector<polygon_t> placed(parts.size());
    for (size_t i = 0; i < parts.size(); i++) {
        auto p = parts[i]->toPolygon();
        if (!p || p->empty())
            continue;
        origPolys[i] = *p;
        placed[i] = computePlacedPolygon(origPolys[i], placements[i]);
    }

    // 1. Geometry preservation: placement must be a rigid-body transform.
    //    Every edge of the placed polygon must have the same length as the
    //    corresponding edge of the original polygon.
    for (size_t i = 0; i < parts.size(); i++) {
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
    for (size_t i = 0; i < parts.size(); i++) {
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

// Compute the cut-order color pair for a part at nesting depth D with max depth M.
// innerColor is cut before outerColor; lower values are cut earlier.
// Formula: deepest parts start at color 1 (inner) / 2 (outer); each shallower level adds 2.
static pair<color_t, color_t> cutColors(int depth, int maxDepth) {
    int base = 2 * (maxDepth - depth);
    return {base + 1, base + 2}; // {innerColor, outerColor}
}

void Nester::write(shared_ptr<FileWriter> writer) const {
    // Fallback to simple horizontal row if run() has not been called.
    if (placements.empty() || placements.size() != parts.size()) {
        long double offset = 0.0;
        const long double spacing = 0.5;
        for (size_t i = 0; i < parts.size(); i++) {
            NesterPart_p p = parts[i];
            BoundingBox bb = p->getBoundingBox();
            long double angle = 0.0;
            long double width = bb.width();
            long double xCorrection = -bb.minX;
            long double yCorrection = -bb.minY;
            if (bb.width() > bb.height()) {
                angle = -90.0;
                width = bb.height();
                xCorrection = bb.maxY;
                yCorrection = -bb.minX;
            }
            transformer_t transformer = makeTransformation(
                (double)angle, (double)(xCorrection + offset), (double)yCorrection);
            // All parts on a row share depth 0; use colors 1 (inner) and 2 (outer).
            writer->beginGroup("part_" + to_string(i));
            p->write(writer, transformer, 2, 1);
            writer->endGroup();
            offset += spacing + width;
        }
        return;
    }

    // Compute nesting depth of each part (length of hostPartIndex chain).
    vector<int> depths(parts.size(), 0);
    int maxDepth = 0;
    for (size_t i = 0; i < parts.size(); i++) {
        int d = 0;
        int host = placements[i].hostPartIndex;
        while (host >= 0) {
            d++;
            host = placements[host].hostPartIndex;
        }
        depths[i] = d;
        maxDepth = max(maxDepth, d);
    }

    for (size_t i = 0; i < parts.size(); i++) {
        auto poly = parts[i]->toPolygon();
        if (!poly || poly->empty())
            continue;
        const Placement &pl = placements[i];
        transformer_t t = computePlacementTransformer(*poly, pl);
        auto [innerColor, outerColor] = cutColors(depths[i], maxDepth);
        writer->beginGroup("part_" + to_string(i));
        parts[i]->write(writer, t, outerColor, innerColor);
        writer->endGroup();
    }
}

} // namespace nester
