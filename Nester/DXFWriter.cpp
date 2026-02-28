#include "DXFWriter.hpp"
#include "Units.hpp"

namespace nester {

	DXFWriter::DXFWriter(string filename) {
		dxf.begin(filename);
	}

	DXFWriter::~DXFWriter() {
		dxf.end();
	}

	void DXFWriter::line(point_t p1, point_t p2, color_t color) {
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