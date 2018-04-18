#ifndef _SVG_WRITER_H_
#define _SVG_WRITER_H_

#include <map>

#include "Nester.hpp"
#include "../XDxfGen/include/xdxfgen.h"

namespace nester {

	class SVGWriter : public FileWriter
	{
		ofstream out;
		std::map<color_t, std::string> colormap;
	public:
		SVGWriter(string filename);
		virtual ~SVGWriter();

		void begin(string filename);
		void end();

		virtual void line(point_t p1, point_t p2, color_t color = 0);
	};

}
 
#endif