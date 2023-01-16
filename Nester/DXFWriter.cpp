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

		dxf.line((double)(p1.x()*mm), (double)(p1.y()*mm),
			(double)(p2.x()*mm), (double)(p2.y()*mm),
			0.0,
			0, // layer
			color);


	}

}