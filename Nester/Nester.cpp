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

	transformer_t makeTransformation(LongDouble x1, LongDouble y1, LongDouble angle, LongDouble x2, LongDouble y2) {
		trans::translate_transformer<LongDouble, 2, 2> translate1(x1, y1);
		trans::rotate_transformer<bg::degree, LongDouble, 2, 2> rotate(angle);
		trans::translate_transformer<LongDouble, 2, 2> translate2(x2, y2);

		//trans::matrix_transformer<LongDouble, 2, 2> translateRotate(translate1.matrix() * rotate.matrix() * translate2.matrix());
		trans::matrix_transformer<LongDouble, 2, 2> translateRotate(translate1.matrix() * translate2.matrix());
		return translateRotate;
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
		for (NesterPart_p p : parts) {
			BoundingBox bb = p->getBoundingBox();
			LongDouble angle = 0.0;
			LongDouble width = bb.width();
			LongDouble xCorrection = -bb.minX;
			LongDouble yCorrection = -bb.minY;

			if (bb.width() > bb.height()) {
				angle = -90.0;
				width = bb.height();
				xCorrection = 0.0;
				yCorrection = bb.maxY;
			}

			
			transformer_t null_transformer = makeTransformation(0.0, 0.0,
				-90.0,                // rotate around origin
				0.0, 0.0);
			p->write(writer, null_transformer); 

			//for (double angle = 0.0; angle < 360.0 ; angle += 45.0) 
			{
				transformer_t transformer = makeTransformation(	0.0, 0.0, 
																angle,                // rotate around origin
																xCorrection, yCorrection);            // move so that new bottom left corner is at y=0 and x=offset

				p->write(writer, transformer);
			}

			*log << "offset=" << offset << " bb[x=(" << bb.minX << "," << bb.maxX << ") y=(" << bb.minY << "," << bb.maxY << ") angle=" << angle << " width=" << width << " correction=(" << xCorrection << "," << yCorrection << ")" << endl;
			offset += spacing + width;
		}

	}

}