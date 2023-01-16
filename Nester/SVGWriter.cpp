#include "SVGWriter.hpp"

namespace nester {


	SVGWriter::SVGWriter(string filename)
	{
		begin(filename);
		colormap[DXF_OUTER_CUT_COLOR] = "black";
		colormap[DXF_INNER_CUT_COLOR] = "red";
		colormap[DXF_DEBUG_COLOR] = "green";
	}

	SVGWriter::~SVGWriter()
	{
		end();
	}

	void SVGWriter::begin(string filename) {
		out.open(filename);
		out << "<?xml version = \"1.0\" encoding = \"UTF-8\" ?>" << endl;
		out << "<svg xmlns = \"http://www.w3.org/2000/svg\" version = \"1.1\">" << endl;
	}

	void SVGWriter::end() {
		out << "</svg>" << endl;
		out.close();
	}

	void SVGWriter::line(point_t p1, point_t p2, color_t color) {
		string colorname = "purple";

		auto search = colormap.find(color);
		if (search != colormap.end()) {
			colorname = search->second;
		}

		out << "<line x1=\"" << p1.x() << "cm\" y1=\"" << p1.y() << "cm\" x2=\"" << p2.x() << "cm\" y2=\"" << p2.y() << "cm\" stroke=\"" << colorname << "\" stroke-width=\"1\" />" << endl;
	}

}