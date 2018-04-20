#include <algorithm> 

#define LIBNFP_PROTOTYPES_IMPLEMENTATION
#include "../libnfporb/libnfp.hpp"
#include "Nester.hpp"

using namespace libnfp;

namespace nester {

	// from https://stackoverflow.com/questions/11826554/standard-no-op-output-stream
	class NullBuffer : public std::streambuf
	{
	public:
		int overflow(int c) { return c; }
	};

	class NullStream : public std::ostream { 
		public: 
			NullStream() : std::ostream(&m_sb) {} 
		private: 
			NullBuffer m_sb; 
	};

	transformer_t makeTransformation(LongDouble angle, LongDouble x, LongDouble y) {
		trans::rotate_transformer<bg::degree, LongDouble, 2, 2> rotate(angle);
		trans::translate_transformer<LongDouble, 2, 2> translate(x, y);

		return translate.matrix() * rotate.matrix();
	}

	void NesterLine::setStartPoint(point_t p) {
		start = p;
	}

	void NesterLine::setEndPoint(point_t p) {
		end = p;
	}

	void NesterLine::write(FileWriter& writer, color_t color, transformer_t& transformer) const {
		point_t tStart, tEnd;

		transformer.apply(start, tStart);
		transformer.apply(end, tEnd);

		writer.line(tStart, tEnd, color);
	}

	BoundingBox NesterLine::getBoundingBox() const {
		BoundingBox bb;
		bb.minX = min(start.x_.val(), end.x_.val());
		bb.minY = min(start.y_.val(), end.y_.val());
		bb.maxX = max(start.x_.val(), end.x_.val());
		bb.maxY = max(start.y_.val(), end.y_.val());

		return bb;
	}

	void NesterNurbs::addControlPoint(double x, double y) {
		controlPoints.push_back(point_t(x, y));
	}

	void NesterNurbs::addKnots(vector<double> ks) {
		knots.insert(knots.end(), ks.begin(), ks.end()); 
	};

	void NesterNurbs::write(FileWriter& writer, color_t color, transformer_t& transformer) const {
		// Not implemented
	}

	BoundingBox NesterNurbs::getBoundingBox() const {
		BoundingBox bb;
		// Not implemented
		return bb;
	}

	void NesterLoop::addEdge(NesterEdge_p edge) {
		edges.push_back(edge);
	}

	void NesterLoop::write(FileWriter& writer, color_t color, transformer_t& transformer) const {
		for (NesterEdge_p edge : edges) {
			edge->write(writer, color, transformer);
		}
	}

	BoundingBox NesterLoop::getBoundingBox() const {
		BoundingBox bb;
		for (NesterEdge_p edge : edges) {
			bb.join(edge->getBoundingBox());
		}

		return bb;
	}

	void NesterPart::setOuterRing(NesterRing_p ring) {
		outer_ring = ring;
	}

	void NesterPart::addInnerRing(NesterRing_p ring) {
		inner_rings.push_back(ring);
	}

	void NesterPart::write(FileWriter& writer, transformer_t& transformer) const {
		outer_ring->write(writer, DXF_OUTER_CUT_COLOR, transformer);
		for (NesterRing_p r : inner_rings) {
			r->write(writer, DXF_INNER_CUT_COLOR, transformer);
		}
	}

	BoundingBox NesterPart::getBoundingBox() const {
		BoundingBox bb;
		bb.join(outer_ring->getBoundingBox());
		for (NesterRing_p r : inner_rings) {
			bb.join(r->getBoundingBox());
		}

		return bb;
	}

	Nester::Nester() {
		log = make_shared<NullStream>();
	}

	void Nester::addPart(NesterPart_p part) {
		parts.push_back(part);
	}

	
	void Nester::run() {
		// convert to polygon rings

	}

	void Nester::write(FileWriter& writer) const {

		LongDouble offset = 0.0;
		const LongDouble spacing = 0.5;

		//writer.line(point_t(-100.0, 0.0), point_t(100.0, 0.0), DXF_DEBUG_COLOR);
		//writer.line(point_t(0.0, -100.0), point_t(0.0, 100.0), DXF_DEBUG_COLOR);

		for (NesterPart_p p : parts) {
			BoundingBox bb = p->getBoundingBox();
			LongDouble angle = 0.0;
			LongDouble width = bb.width();
			LongDouble xCorrection = -bb.minX;
			LongDouble yCorrection = -bb.minY;

			if (bb.width() > bb.height()) {
				angle = -90.0;
				width = bb.height();
				xCorrection = bb.maxY;
				yCorrection = -bb.minX;
			}

			
			transformer_t transformer = makeTransformation(	
															angle,                // rotate around origin
															xCorrection + offset, yCorrection);            // move so that new bottom left corner is at y=0 and x=offset

			p->write(writer, transformer);
			

			*log << "offset=" << offset << " bb[x=(" << bb.minX << "," << bb.maxX << ") y=(" << bb.minY << "," << bb.maxY << ") angle=" << angle << " width=" << width << " correction=(" << xCorrection << "," << yCorrection << ")" << endl;
			offset += spacing + width;
		}

	}

}
