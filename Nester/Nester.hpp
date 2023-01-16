#ifndef _NESTER_H
#define _NESTER_H

#include <memory>
#include <vector>

#include <boost/geometry.hpp>
#include <boost/geometry/strategies/transform/matrix_transformers.hpp>

#include "../XDxfGen/include/xdxfgen.h"


using namespace std;

namespace bg = boost::geometry;
namespace trans = bg::strategy::transform;

namespace nester {

	typedef int color_t;
	const color_t DXF_OUTER_CUT_COLOR = 1;
	const color_t DXF_INNER_CUT_COLOR = 2;
	const color_t DXF_DEBUG_COLOR = 3;

	typedef bg::model::d2::point_xy<long double> point_t;
	typedef bg::model::polygon<point_t> polygon_t;

	typedef shared_ptr<polygon_t> polygon_p;

	typedef trans::matrix_transformer<long double, 2, 2> transformer_t;
	extern transformer_t makeTransformation(long double angle, long double x, long double y);

	struct BoundingBox {
		long double minX, minY;
		long double maxX, maxY;

		BoundingBox() :
			minX(std::numeric_limits<long double>::infinity()),
			minY(std::numeric_limits<long double>::infinity()),
			maxX(-std::numeric_limits<long double>::infinity()),
			maxY(-std::numeric_limits<long double>::infinity()) {}

		long double width() const {
			return maxX - minX;
		}

		long double height() const {
			return maxY - minY;
		}

		void join(BoundingBox other) {
			minX = min(minX, other.minX);
			maxX = max(maxX, other.maxX);
			minY = min(minY, other.minY);
			maxY = max(maxY, other.maxY);
		}
	};

    class FileWriter {
        public:
          virtual void line(point_t p1, point_t p2, int color = 0) = 0;
    };
     

	class NesterEdge {
	public:
		virtual void write(shared_ptr<FileWriter> writer, color_t color, transformer_t& transformer) const = 0;
		virtual BoundingBox getBoundingBox() const = 0;
	};

	typedef shared_ptr<NesterEdge> NesterEdge_p;

	class NesterNurbs : public NesterEdge {
		vector<point_t> controlPoints;
		vector<double> knots;
	public:
		void addControlPoint(double x, double y);
		void addKnots(vector<double> knobs);

		virtual void write(shared_ptr<FileWriter> writer, color_t color, transformer_t& transformer) const;
		virtual BoundingBox getBoundingBox() const;
	};

	class NesterLine : public NesterEdge {
		point_t start, end;
	public:
		void setStartPoint(point_t p);
		void setEndPoint(point_t p);

		virtual void write(shared_ptr<FileWriter> writer, color_t color, transformer_t& transformer) const;
		virtual BoundingBox getBoundingBox() const;
	};

	// A ring is a closed line (either a loop of segments, a circle or an ellipse)
	class NesterRing {
	public:
		virtual void write(shared_ptr<FileWriter> writer, color_t color, transformer_t& transformer) const = 0;
		virtual BoundingBox getBoundingBox() const = 0;
	};

	typedef shared_ptr<NesterRing> NesterRing_p;

	class NesterLoop : public NesterRing {
		vector<NesterEdge_p> edges;
	public:
		void addEdge(NesterEdge_p primitive);
		virtual void write(shared_ptr<FileWriter> writer, color_t color, transformer_t& transformer) const;
		virtual BoundingBox getBoundingBox() const;
	};

	// A part has an outer boundary and zero or more inner boundaries (holes)
	class NesterPart {
		NesterRing_p outer_ring;
		vector<NesterRing_p> inner_rings;
	public:
		void setOuterRing(NesterRing_p loop);
		void addInnerRing(NesterRing_p loop);

		polygon_p toPolygon() const;
		virtual void write(shared_ptr<FileWriter> writer, transformer_t& transformer) const;
		virtual BoundingBox getBoundingBox() const;
	};

	typedef shared_ptr<NesterPart> NesterPart_p;

	class Nester {
		vector<NesterPart_p> parts;
		shared_ptr<ostream> log;
	public:
		Nester();

		void addPart(NesterPart_p part);

		void run();

		void write(shared_ptr<FileWriter> writer) const;
	};

}

#endif
