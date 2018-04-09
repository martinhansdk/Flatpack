#ifndef _NESTER_H
#define _NESTER_H

#include <memory>
#include <vector>

using namespace std;

namespace nester {

	class NesterEdge {

	};

	typedef shared_ptr<NesterEdge> NesterEdge_p;

	class NesterNurbs : public NesterEdge {
	public:
		void addControlPoint(double x, double y);
		void addKnots(vector<double> knobs);
	};

	// A ring is a closed line (either a loop of segments, a circle or an ellipse)
	class NesterRing {

	};

	typedef shared_ptr<NesterRing> NesterRing_p;

	class NesterLoop : public NesterRing {
		vector<NesterEdge_p> edges;
	public:
		void addEdge(NesterEdge_p primitive);
	};

	// A part has an outer boundary and zero or more inner boundaries (holes)
	class NesterPart {
		NesterRing_p outer_ring;
		vector<NesterRing_p> inner_rings;
	public:
		void addRing(NesterRing_p loop);
	};

	typedef shared_ptr<NesterPart> NesterPart_p;

	class Nester {
		vector<NesterPart_p> parts;
	public:
		void addPart(NesterPart_p part);
	};

}

#endif