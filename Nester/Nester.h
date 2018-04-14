#ifndef _NESTER_H
#define _NESTER_H

#include <memory>
#include <vector>

#include "../libnfporb/libnfp.hpp"
#include "../XDxfGen/include/xdxfgen.h"

using namespace std;
using namespace libnfp;

namespace nester {

	typedef XDxfGen<double> dxfwriter_t;
	typedef int dxf_color_t;
	const dxf_color_t DXF_OUTER_CUT_COLOR = 1;
	const dxf_color_t DXF_INNER_CUT_COLOR = 2;

	typedef shared_ptr<polygon_t> polygon_p;

	class NesterEdge {
	public:
		virtual void writeDXF(dxfwriter_t& writer, dxf_color_t color) const = 0;
	};

	typedef shared_ptr<NesterEdge> NesterEdge_p;

	class NesterNurbs : public NesterEdge {
		vector<point_t> controlPoints;
		vector<double> knots;
	public:
		void addControlPoint(double x, double y);
		void addKnots(vector<double> knobs);

		virtual void writeDXF(dxfwriter_t& writer, dxf_color_t color) const;
	};

	class NesterLine : public NesterEdge {
		point_t start, end;
	public:
		void setStartPoint(point_t p);
		void setEndPoint(point_t p);

		virtual void writeDXF(dxfwriter_t& writer, dxf_color_t color) const;
	};

	// A ring is a closed line (either a loop of segments, a circle or an ellipse)
	class NesterRing {
	public:
		virtual void writeDXF(dxfwriter_t& writer, dxf_color_t color) const = 0;
	};

	typedef shared_ptr<NesterRing> NesterRing_p;

	class NesterLoop : public NesterRing {
		vector<NesterEdge_p> edges;
	public:
		void addEdge(NesterEdge_p primitive);
		virtual void writeDXF(dxfwriter_t& writer, dxf_color_t color) const;
	};

	// A part has an outer boundary and zero or more inner boundaries (holes)
	class NesterPart {
		NesterRing_p outer_ring;
		vector<NesterRing_p> inner_rings;
	public:
		void addRing(NesterRing_p loop);

		polygon_p toPolygon() const;
		virtual void writeDXF(dxfwriter_t& writer) const;
	};

	typedef shared_ptr<NesterPart> NesterPart_p;

	class Nester {
		vector<NesterPart_p> parts;
	public:
		void addPart(NesterPart_p part);

		void run();

		void writeDXF(string filename) const;
	};

}

#endif