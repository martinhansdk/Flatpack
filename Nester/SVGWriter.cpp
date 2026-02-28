#include <cmath>
#include <cstdio>

#include "SVGWriter.hpp"

namespace nester {

    SVGWriter::SVGWriter(string filename)
    {
        begin(filename);
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

    // Golden-angle HSL: successive levels get maximally distinct hues.
    // S=0.75, L=0.40 keeps colors saturated and well away from white.
    string SVGWriter::colorFromLevel(int level) {
        double hue = fmod((level - 1) * 137.508, 360.0);
        double s = 0.75, l = 0.40;

        // HSL -> RGB
        double c = (1.0 - fabs(2.0 * l - 1.0)) * s;
        double x = c * (1.0 - fabs(fmod(hue / 60.0, 2.0) - 1.0));
        double m = l - c / 2.0;
        double r, g, b;
        if      (hue < 60)  { r = c; g = x; b = 0; }
        else if (hue < 120) { r = x; g = c; b = 0; }
        else if (hue < 180) { r = 0; g = c; b = x; }
        else if (hue < 240) { r = 0; g = x; b = c; }
        else if (hue < 300) { r = x; g = 0; b = c; }
        else                { r = c; g = 0; b = x; }

        int ri = (int)round((r + m) * 255);
        int gi = (int)round((g + m) * 255);
        int bi = (int)round((b + m) * 255);
        char buf[8];
        snprintf(buf, sizeof(buf), "#%02x%02x%02x", ri, gi, bi);
        return string(buf);
    }

    void SVGWriter::line(point_t p1, point_t p2, color_t color) {
        string colorname = colorFromLevel(color > 0 ? color : 1);
        out << "<line x1=\"" << p1.x << "cm\" y1=\"" << p1.y
            << "cm\" x2=\"" << p2.x << "cm\" y2=\"" << p2.y
            << "cm\" stroke=\"" << colorname << "\" stroke-width=\"1\" />" << endl;
    }

    void SVGWriter::beginGroup(const string &id) {
        out << "<g id=\"" << id << "\">" << endl;
    }

    void SVGWriter::endGroup() {
        out << "</g>" << endl;
    }

}
