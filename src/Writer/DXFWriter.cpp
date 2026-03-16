#include "DXFWriter.hpp"

// convert from cm by multiplying a constant
constexpr double mm = 10.0;

namespace writer {

	using namespace deepnest;

	DXFWriter::DXFWriter(string filename) {
		dxf.begin(filename);
	}

	DXFWriter::~DXFWriter() {
		dxf.end();
	}

	void DXFWriter::line(Point p1, Point p2, Color color) {
        // Map cut-order level to ACI 1-6 (red, yellow, green, cyan, blue, magenta).
        // ACI 1-6 are always visually distinct in CAD/CAM software.
        int aci = ((color - 1) % 6) + 1;

		dxf.line((double)(p1.x*mm), (double)(p1.y*mm),
			(double)(p2.x*mm), (double)(p2.y*mm),
			0.0,
			0, // layer
			aci);
	}

}