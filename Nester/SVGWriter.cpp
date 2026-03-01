#include <cmath>
#include <cstdio>

#include "SVGWriter.hpp"

namespace nester {

    // Named CSS colors for cut-order levels, cycling if there are more levels
    // than colors. Named colors are accepted by all SVG validators and laser
    // cutter tools; computed hex values are often rejected by strict profiles.
    static const char *const namedColors[] = {
        "black", "red", "blue", "green",
        "purple", "orange", "brown", "teal",
        "maroon", "darkviolet", "darkorange", "darkgreen",
    };
    static const int NUM_NAMED_COLORS = 12;

    string SVGWriter::colorFromLevel(int level) {
        return namedColors[(level - 1) % NUM_NAMED_COLORS];
    }

    SVGWriter::SVGWriter(string filename)
        : minX_(std::numeric_limits<double>::infinity()),
          minY_(std::numeric_limits<double>::infinity()),
          maxX_(-std::numeric_limits<double>::infinity()),
          maxY_(-std::numeric_limits<double>::infinity())
    {
        begin(filename);
    }

    SVGWriter::~SVGWriter()
    {
        end();
    }

    void SVGWriter::begin(string filename) {
        out.open(filename);
        // Reset buffer and bounding box so the writer can be reused.
        body_.str("");
        body_.clear();
        minX_ = std::numeric_limits<double>::infinity();
        minY_ = std::numeric_limits<double>::infinity();
        maxX_ = -std::numeric_limits<double>::infinity();
        maxY_ = -std::numeric_limits<double>::infinity();
        // Header is deferred to end() once the bounding box is known.
    }

    void SVGWriter::end() {
        // Write the XML declaration and SVG root element with explicit physical
        // size and viewBox. Coordinates use raw numbers where 1 unit = 1 cm,
        // so width/height in cm match the coordinate range exactly.
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";

        if (minX_ < maxX_ && minY_ < maxY_) {
            double pad = (maxX_ - minX_ + maxY_ - minY_) * 0.02 + 0.5;
            double vx = minX_ - pad, vy = minY_ - pad;
            double vw = (maxX_ - minX_) + 2 * pad;
            double vh = (maxY_ - minY_) + 2 * pad;
            out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\""
                << " width=\"" << vw << "cm\" height=\"" << vh << "cm\""
                << " viewBox=\"" << vx << " " << vy << " " << vw << " " << vh << "\">\n";
        } else {
            // Empty document — emit a minimal valid SVG.
            out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\""
                << " width=\"1cm\" height=\"1cm\" viewBox=\"0 0 1 1\">\n";
        }

        out << body_.str();
        out << "</svg>\n";
        out.close();
    }

    void SVGWriter::line(point_t p1, point_t p2, color_t color) {
        string colorname = colorFromLevel(color > 0 ? color : 1);
        // Coordinates are raw numbers (1 unit = 1 cm); no unit suffix needed
        // because the viewBox establishes the mapping.
        // stroke-width 0.02 = 0.2 mm, a typical laser kerf width for preview.
        body_ << "<line x1=\"" << p1.x << "\" y1=\"" << p1.y
              << "\" x2=\"" << p2.x << "\" y2=\"" << p2.y
              << "\" stroke=\"" << colorname << "\" stroke-width=\"0.02\"/>\n";
        updateBB(p1);
        updateBB(p2);
    }

    void SVGWriter::beginGroup(const string &id) {
        body_ << "<g id=\"" << id << "\">\n";
    }

    void SVGWriter::endGroup() {
        body_ << "</g>\n";
    }

    // ---- SVGStringWriter ----

    // Coordinates are written as raw numbers (1 unit = 1 cm). A viewBox is
    // computed from the bounding box so the SVG scales to fit any container.
    string SVGStringWriter::toString() const {
        ostringstream out;
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";

        if (minX < maxX && minY < maxY) {
            double w = maxX - minX;
            double h = maxY - minY;
            double pad = (w + h) * 0.03 + 0.5; // 3% + 0.5 cm margin
            double vx = minX - pad, vy = minY - pad;
            double vw = w + 2 * pad, vh = h + 2 * pad;
            out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\""
                << " viewBox=\"" << vx << " " << vy << " " << vw << " " << vh << "\""
                << " style=\"width:100%;height:100%;display:block\">\n";
        } else {
            out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n";
        }

        out << body.str();
        out << "</svg>\n";
        return out.str();
    }

    void SVGStringWriter::line(point_t p1, point_t p2, color_t color) {
        string colorname = SVGWriter::colorFromLevel(color > 0 ? color : 1);
        // Use raw numbers (no unit suffix) so viewBox controls the scale.
        // stroke-width 0.05 ≈ 0.5 mm at 1 unit = 1 cm, suitable for a preview.
        body << "<line x1=\"" << p1.x << "\" y1=\"" << p1.y
             << "\" x2=\"" << p2.x << "\" y2=\"" << p2.y
             << "\" stroke=\"" << colorname << "\" stroke-width=\"0.05\"/>\n";
        updateBB(p1);
        updateBB(p2);
    }

    void SVGStringWriter::beginGroup(const string &id) {
        body << "<g id=\"" << id << "\">\n";
    }

    void SVGStringWriter::endGroup() {
        body << "</g>\n";
    }

}
