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

		dxf.line((double)(p1.x_.val()*mm), (double)(p1.y_.val()*mm),
			(double)(p2.x_.val()*mm), (double)(p2.y_.val()*mm),
			0.0,
			0, // layer
			color);


	}

}