# include "Nester.h"

namespace nester {

	double toMM(long double cm) {
		return cm * 10.0;
	}

	void NesterLine::setStartPoint(point_t p) {
		start = p;
	}

	void NesterLine::setEndPoint(point_t p) {
		end = p;
	}

	void NesterLine::writeDXF(dxfwriter_t& writer, dxf_color_t color) const {
		writer.line(
			(double)toMM(start.x_.val()), (double)toMM(start.y_.val()), 
			(double)toMM(end.x_.val()), (double)toMM(end.y_.val()),
			0.0,
			0, // layer
			color);
	}

	void NesterNurbs::addControlPoint(double x, double y) {
		controlPoints.push_back(point_t(x, y));
	}

	void NesterNurbs::addKnots(vector<double> ks) {
		knots.insert(knots.end(), ks.begin(), ks.end()); 
	};

	void NesterNurbs::writeDXF(dxfwriter_t& writer, dxf_color_t color) const {
		// Not implemented
	}

	void NesterLoop::addEdge(NesterEdge_p edge) {
		edges.push_back(edge);
	}

	void NesterLoop::writeDXF(dxfwriter_t& writer, dxf_color_t color) const {
		for (NesterEdge_p edge : edges) {
			edge->writeDXF(writer, color);
		}
	}

	void NesterPart::setOuterRing(NesterRing_p ring) {
		outer_ring = ring;
	}

	void NesterPart::addInnerRing(NesterRing_p ring) {
		inner_rings.push_back(ring);
	}

	void NesterPart::writeDXF(dxfwriter_t& writer) const {
		outer_ring->writeDXF(writer, DXF_OUTER_CUT_COLOR);
		for (NesterRing_p r : inner_rings) {
			r->writeDXF(writer, DXF_INNER_CUT_COLOR);
		}
	}

	void Nester::addPart(NesterPart_p part) {
		parts.push_back(part);
	}

	
	void Nester::run() {
		// convert to polygon rings

	}

	void Nester::writeDXF(string filename) const {
		dxfwriter_t dxf;

		dxf.begin(filename);

		for (NesterPart_p p : parts) {
			p->writeDXF(dxf);
		}

		dxf.end();
	}

}