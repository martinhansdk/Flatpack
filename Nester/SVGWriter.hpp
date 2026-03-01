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
        // Line content is buffered so the bounding box is known before the
        // header is written. The header (with viewBox) is emitted in end().
        ostringstream body_;
        double minX_, minY_, maxX_, maxY_;

        void updateBB(point_t p) {
            if (p.x < minX_) minX_ = p.x;
            if (p.y < minY_) minY_ = p.y;
            if (p.x > maxX_) maxX_ = p.x;
            if (p.y > maxY_) maxY_ = p.y;
        }

    public:
        SVGWriter(string filename);
        virtual ~SVGWriter();

        void begin(string filename);
        void end();

        virtual void line(point_t p1, point_t p2, color_t color = 0) override;
        virtual void beginGroup(const string &id) override;
        virtual void endGroup() override;

        // Returns a named CSS color for cut-order level `level` (1-based).
        // Cycles through a fixed list of visually distinct, universally
        // supported named colors so the output is accepted by strict validators.
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
