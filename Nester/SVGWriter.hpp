#ifndef _SVG_WRITER_H_
#define _SVG_WRITER_H_

#include <limits>
#include <sstream>

#include "Nester.hpp"
#include "../XDxfGen/include/xdxfgen.h"

namespace nester {

    class SVGWriter : public FileWriter
    {
        ofstream out;
    public:
        SVGWriter(string filename);
        virtual ~SVGWriter();

        void begin(string filename);
        void end();

        virtual void line(point_t p1, point_t p2, color_t color = 0) override;
        virtual void beginGroup(const string &id) override;
        virtual void endGroup() override;

        // Returns an SVG color string for cut-order level `level` (1-based).
        // Uses a golden-angle HSL distribution: visually distinct, never near white.
        static string colorFromLevel(int level);
    };

    // Writes SVG to an in-memory string instead of a file.
    // Uses unitless coordinates (1 unit = 1 cm) and a computed viewBox so the
    // result scales correctly when displayed inline or in an <img>/<object> tag.
    // Call toString() after write() to get the complete SVG document.
    class SVGStringWriter : public FileWriter {
        ostringstream body;
        double minX, minY, maxX, maxY;

        void updateBB(point_t p) {
            if (p.x < minX) minX = p.x;
            if (p.y < minY) minY = p.y;
            if (p.x > maxX) maxX = p.x;
            if (p.y > maxY) maxY = p.y;
        }

      public:
        SVGStringWriter()
            : minX(std::numeric_limits<double>::infinity()),
              minY(std::numeric_limits<double>::infinity()),
              maxX(-std::numeric_limits<double>::infinity()),
              maxY(-std::numeric_limits<double>::infinity()) {}

        string toString() const;

        virtual void line(point_t p1, point_t p2, color_t color = 0) override;
        virtual void beginGroup(const string &id) override;
        virtual void endGroup() override;
    };

}

#endif
