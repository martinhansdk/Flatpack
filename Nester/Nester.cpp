# include "Nester.h"

namespace nester {

	void NesterLoop::addEdge(NesterEdge_p edge) {
		edges.push_back(edge);
	}


	void NesterPart::addRing(NesterRing_p ring) {
		if (outer_ring == nullptr) {
			outer_ring = ring;
		}
		else {
			inner_rings.push_back(ring);
		}
	}

	void Nester::addPart(NesterPart_p part) {
		parts.push_back(part);
	}

	void addControlPoint(double x, double y) {

	}

	void addKnots(vector<double> knobs) {

	}
}