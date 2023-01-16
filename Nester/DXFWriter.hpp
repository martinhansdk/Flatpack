#ifndef _DXFWRITER_H_
#define _DXFWRITER_H_

#include "Nester.hpp"
#include "../XDxfGen/include/xdxfgen.h"

namespace nester {

	class DXFWriter : public FileWriter {
		XDxfGen<long double> dxf;
	public:
		DXFWriter(string filename);
		virtual ~DXFWriter();
		virtual void line(point_t p1, point_t p2, color_t color = 0);
	};

}

#endif
