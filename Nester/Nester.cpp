# include "Nester.h"

#include "../libnfporb/libnfp_implementation.hpp"

namespace nester {

	void NesterNurbs::addControlPoint(double x, double y) {
		controlPoints.push_back(point_t(x, y));
	}

	void NesterNurbs::addKnots(vector<double> ks) {
		knots.insert(knots.end(), ks.begin(), ks.end()); 
	};

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

	
	void Nester::run() {
		// convert to polygon rings

	}

}