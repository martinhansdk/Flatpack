#ifndef _NESTER_H
#define _NESTER_H

#include <functional>
#include <limits>
#include <memory>
#include <vector>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>

#include "../XDxfGen/include/xdxfgen.h"

using namespace std;

namespace nester {

typedef int color_t;
const color_t DXF_OUTER_CUT_COLOR = 1;
const color_t DXF_INNER_CUT_COLOR = 2;
const color_t DXF_DEBUG_COLOR = 3;

typedef glm::dvec2 point_t;
typedef std::vector<point_t> polygon_t;

typedef shared_ptr<polygon_t> polygon_p;

typedef glm::dmat3 transformer_t;
extern transformer_t makeTransformation(double angle, double x, double y);

struct BoundingBox {
    long double minX, minY;
    long double maxX, maxY;

    BoundingBox()
        : minX(std::numeric_limits<long double>::infinity()),
          minY(std::numeric_limits<long double>::infinity()),
          maxX(-std::numeric_limits<long double>::infinity()),
          maxY(-std::numeric_limits<long double>::infinity()) {}

    long double width() const { return maxX - minX; }

    long double height() const { return maxY - minY; }

    void join(BoundingBox other) {
        minX = min(minX, other.minX);
        maxX = max(maxX, other.maxX);
        minY = min(minY, other.minY);
        maxY = max(maxY, other.maxY);
    }
};

// Geometry helpers (in nester namespace, declared here for testability)
polygon_t transformPolygon(const polygon_t &poly, transformer_t t);
BoundingBox computeBB(const polygon_t &poly);
bool segmentsIntersect(point_t a1, point_t a2, point_t b1, point_t b2);
bool pointInPolygon(point_t p, const polygon_t &poly);
bool polygonsOverlap(const polygon_t &a, const polygon_t &b);
double polygonMinDistance(const polygon_t &a, const polygon_t &b);

class FileWriter {
  public:
    virtual void line(point_t p1, point_t p2, int color = 0) = 0;
};

class NesterEdge {
  public:
    virtual void write(shared_ptr<FileWriter> writer, color_t color,
                       transformer_t &transformer) const = 0;
    virtual BoundingBox getBoundingBox() const = 0;
};

typedef shared_ptr<NesterEdge> NesterEdge_p;

class NesterNurbs : public NesterEdge {
    vector<point_t> controlPoints;
    vector<double> knots;

  public:
    void addControlPoint(double x, double y);
    void addKnots(vector<double> knobs);

    virtual void write(shared_ptr<FileWriter> writer, color_t color,
                       transformer_t &transformer) const;
    virtual BoundingBox getBoundingBox() const;
};

class NesterLine : public NesterEdge {
    point_t start, end;

  public:
    void setStartPoint(point_t p);
    void setEndPoint(point_t p);
    point_t getStartPoint() const;

    virtual void write(shared_ptr<FileWriter> writer, color_t color,
                       transformer_t &transformer) const;
    virtual BoundingBox getBoundingBox() const;
};

// A ring is a closed line (either a loop of segments, a circle or an ellipse)
class NesterRing {
  public:
    virtual void write(shared_ptr<FileWriter> writer, color_t color,
                       transformer_t &transformer) const = 0;
    virtual BoundingBox getBoundingBox() const = 0;
    virtual polygon_p toPolygon() const = 0;
};

typedef shared_ptr<NesterRing> NesterRing_p;

class NesterLoop : public NesterRing {
    vector<NesterEdge_p> edges;

  public:
    void addEdge(NesterEdge_p primitive);
    virtual void write(shared_ptr<FileWriter> writer, color_t color,
                       transformer_t &transformer) const;
    virtual BoundingBox getBoundingBox() const;
    virtual polygon_p toPolygon() const override;
};

// A part has an outer boundary and zero or more inner boundaries (holes)
class NesterPart {
    NesterRing_p outer_ring;
    vector<NesterRing_p> inner_rings;

  public:
    void setOuterRing(NesterRing_p loop);
    void addInnerRing(NesterRing_p loop);

    polygon_p toPolygon() const;
    vector<polygon_p> toHolePolygons() const;
    virtual void write(shared_ptr<FileWriter> writer, transformer_t &transformer) const;
    virtual BoundingBox getBoundingBox() const;
};

typedef shared_ptr<NesterPart> NesterPart_p;

struct Placement {
    double x, y;       // absolute sheet position (cm), bottom-left of rotated bbox
    double angle;      // rotation in degrees (continuous)
    int hostPartIndex; // -1 = on sheet; >=0 = inside hole of this part
    int hostHoleIndex; // which hole (valid when hostPartIndex >= 0)
};

// Apply a Placement to a polygon (rotate then translate so bbox min -> (pl.x, pl.y)).
polygon_t computePlacedPolygon(const polygon_t &poly, const Placement &pl);

class Nester {
    vector<NesterPart_p> parts;
    vector<Placement> placements;
    double kerf_cm;
    shared_ptr<ostream> log;

  public:
    Nester();

    void addPart(NesterPart_p part);
    void setKerf(double k);

    // progress(current, total) is called after each outer SA iteration.
    // Return false from the callback to cancel early (best result so far is used).
    void run(std::function<bool(int, int)> progress = nullptr);
    void write(shared_ptr<FileWriter> writer) const;

    // Returns the placements produced by run().  Empty before run() is called.
    vector<Placement> getPlacements() const;

    // Returns the initial row-layout placements (before SA optimisation).
    // Useful for testing and for comparing against the result of run().
    vector<Placement> computeInitialPlacement() const;
};

} // namespace nester

#endif
