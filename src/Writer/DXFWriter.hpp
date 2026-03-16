#ifndef _DXFWRITER_H_
#define _DXFWRITER_H_

#include "FileWriter.hpp"
#include "../XDxfGen/include/xdxfgen.h"

namespace writer {

	class DXFWriter : public FileWriter {
		XDxfGen<long double> dxf;
	public:
		DXFWriter(string filename);
		virtual ~DXFWriter();
		virtual void line(deepnest::Point p1, deepnest::Point p2, Color color = 0);
	};

}

#endif
