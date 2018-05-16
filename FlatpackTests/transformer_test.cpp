#include <sstream>

#include "catch.hpp"
#include "../libnfporb/libnfp.hpp"
#include "../Nester/Nester.hpp"
#include <boost/geometry/io/wkt/write.hpp>

using namespace nester;
using namespace libnfp;
using namespace std;

namespace NesterTests
{

	string str(point_t p) {
		stringstream s;
		s << std::setprecision(1) << std::fixed << bg::wkt(p);

		return s.str();
	}

	string str(bg::model::box<point_t> box) {
		stringstream s;
		s << std::setprecision(1) << std::fixed << bg::wkt(box);

		return s.str();
	}



	typedef bg::model::multi_polygon<polygon_t> multi_polygon_t;
	typedef bg::model::multi_point<point_t> nfp_mp_t;

	typedef bg::model::multi_point<pointf_t> nfpf_mp_t;

	nfpf_mp_t convert(nfp_mp_t mp)
	{
		nfpf_mp_t pf;

		for (const auto& pt : mp) {
			pf.push_back(pointf_t(toLongDouble(pt.x_), toLongDouble(pt.y_)));
		}
		return pf;
	}

	void write_svg(std::string const& filename, typename multi_polygon_t const& polygons, const nfp_mp_t& nfp)
	{
		std::ofstream svg(filename.c_str());

		boost::geometry::svg_mapper<pointf_t> mapper(svg, 100, 100, "width=\"200mm\" height=\"200mm\" viewBox=\"-250 -250 500 500\"");
		for (auto p : polygons) {
			auto pf = convert(p);
			mapper.add(pf);
			mapper.map(pf, "fill-opacity:0.5;fill:rgb(153,204,0);stroke:rgb(153,204,0);stroke-width:2");
		}

		nfpf_mp_t nfpf = convert(nfp);
		
		mapper.add(nfpf);
		mapper.map(nfpf, "fill-opacity:0.5;fill:rgb(204,153,0);stroke:rgb(204,153,0);stroke-width:2");
		/*
		for (auto& r : nfpf.inners()) {
			if (r.size() == 1) {
				mapper.add(r.front());
				mapper.map(r.front(), "fill-opacity:0.5;fill:rgb(204,153,0);stroke:rgb(204,153,0);stroke-width:2");
			}
			else if (r.size() == 2) {
				segmentf_t seg(r.front(), *(r.begin() + 1));
				mapper.add(seg);
				mapper.map(seg, "fill-opacity:0.5;fill:rgb(204,153,0);stroke:rgb(204,153,0);stroke-width:2");
			}
		}
		*/
	}

	double calculate_cost(const multi_polygon_t& placed_parts) {
		bg::model::box<point_t> bb;
		bg::envelope(placed_parts, bb);
		double width = (double) (bb.max_corner().x_ - bb.min_corner().x_).val();
		double height = (double) (bb.max_corner().y_ - bb.min_corner().y_).val();

		// cost is calculated width + height of bounding box, but with more weight given to width
		return (2.0L * width) + height;
	}

	multi_polygon_t to_multi_polygon(const nfp_t& nfp) {
		multi_polygon_t mp;
		for (polygon_t::ring_type ring : nfp) {
			polygon_t poly;
			bg::assign(poly, ring);
			mp.push_back(poly);
		}
		return mp;
	}

	nfp_mp_t to_multi_point(const nfp_t& nfp) {
		nfp_mp_t mp;
		for (polygon_t::ring_type ring : nfp) {
			for (polygon_t::point_type point : ring) {
				cout << "nfp point " << bg::wkt(point) << endl;
				bg::append(mp, point);
			}
		}
		return mp;
	}

	void prepare_bin_for_nfp(const libnfp::polygon_t &bin, libnfp::polygon_t &bin_with_hole)
	{
		// libnfp does not support calculating an inside nfp, so use it's ability to 
		// generate an nfp of a polygon with a hole. Make a polygon that has the 
		// bin's polygon as an inner hole and a slightly enlarged version of the 
		// bounding box of the bin as as the outer ring.
		boost::geometry::model::box<point_t> bin_bb;
		bg::envelope(bin, bin_bb);
		bin_bb.min_corner().x_ -= 1.0;
		bin_bb.min_corner().y_ -= 1.0;
		bin_bb.max_corner().x_ += 1.0;
		bin_bb.max_corner().y_ += 1.0;
		polygon_t::ring_type bin_bb_ring;
		bg::convert(bin_bb, bin_bb_ring);

		bin_with_hole.outer() = bin_bb_ring;
		bin_with_hole.inners().push_back(bin.outer());
		bg::correct(bin_with_hole);
	}

	char num = 'a';

	void writeDebugSVG(string base_filename, multi_polygon_t geo) {
		string filename = base_filename + "-" + num + ".svg";
		write_svg(filename, geo);
		cout << "filename=" << filename << endl;
	}

	void writeDebugSVG(string base_filename, multi_polygon_t geo, nfp_mp_t nfp) {
		string filename = base_filename + "-" + num + ".svg";
		write_svg(filename, geo, nfp);
		cout << "filename=" << filename << endl;
	}

	// pass bin and part by value because generateNFP() modifies it's arguments
	nfp_mp_t generateBinNfp(polygon_t bin, polygon_t part)
	{
		// libnfp does not support calculating an inside nfp, so use it's ability to 
		// generate an nfp of a polygon with a hole. Generate the nfp of the bin polygon 
		// which has a hole, then include only the part of the nfp that falls inside the bin.
		cout << "generateBinNfp: bin=" << bg::wkt(bin) << " part=" << bg::wkt(part) << endl;
		 
		nfp_t bin_nfp = generateNFP(bin, part, true);
		string filename = string("bin_nfp") + num + ".svg";
		write_svg(filename, { bin, part }, bin_nfp);
		cout << "filename=" << filename << endl;

		nfp_mp_t bin_nfp_mp;
		for (polygon_t::ring_type ring : bin_nfp) {
			if (bg::within(ring, bin.outer())) {
				for (point_t point : ring) {
					bin_nfp_mp.push_back(point);
				}

				cout << "generateBinNfp: " << bg::wkt(ring) << " is within " << bg::wkt(bin) << endl;
			}
			else {
				cout << "generateBinNfp: " << bg::wkt(ring) << " is not within, skip " << bg::wkt(bin) << endl;
			}
		} 

		cout << "bin_nfp_mp=" << bg::wkt(bin_nfp_mp) << endl;

		return bin_nfp_mp;
	}

	inline trans::translate_transformer<LongDouble, 2, 2> make_translate_transformer(LongDouble x, LongDouble y) {
		trans::translate_transformer<LongDouble, 2, 2> translate(x, y);
		return translate;
	}

	point_t find_leftmost_point(const NesterTests::nfp_mp_t &bin_nfp)
	{
		point_t leftmost_position(numeric_limits<LongDouble>::infinity(), numeric_limits<LongDouble>::infinity());

		for (polygon_t::point_type point : bin_nfp) {
			if (point.x_ < leftmost_position.x_) {
				leftmost_position = point;
			}
		}
		
		return leftmost_position;
	}


	polygon_t move_part_to_point(const polygon_t &part, point_t pos) {
		transformer_t transformer = make_translate_transformer(
			pos.x_ - part.outer()[0].x_,
			pos.y_ - part.outer()[0].y_);


		polygon_t placed_part;
		bg::transform(part, placed_part, transformer);
		return placed_part;
	}

	double place(const polygon_t bin, const multi_polygon_t parts, multi_polygon_t &placed_parts, multi_polygon_t &unplaceable_parts) {
		polygon_t bin_for_nfp;
		prepare_bin_for_nfp(bin, bin_for_nfp);
		

		double cost = numeric_limits<double>::infinity();
			
		multi_polygon_t union_of_placed_parts;

		for (polygon_t part : parts) {
			num++;
			cout << "------------" << endl;

			if (placed_parts.size() == 0) {
				// first part, place it at the left of the bin
				cout << "first part!" << endl;
				nfp_mp_t bin_nfp = generateBinNfp(bin_for_nfp, part);

				cout << "bin NFP: " << bg::wkt(bin_nfp) << endl;

				if (bin_nfp.size() == 0) {
					// unplaceable in the bin
					unplaceable_parts.push_back(part);
					cout << "unplaceable: " << bg::wkt(part) << "   in: " << bg::wkt(bin_for_nfp) << endl;
					continue;
				}

				point_t leftmost_position = find_leftmost_point(bin_nfp);

				polygon_t placed_part = move_part_to_point(part, leftmost_position);
				placed_parts.push_back(placed_part);
				union_of_placed_parts.push_back(placed_part);
				cout << "placed at " << bg::wkt(leftmost_position) << endl;
				continue;
			}

			cout << "not the first part!" << endl;
			writeDebugSVG("union_of_placed_parts", union_of_placed_parts);
			// find the nfps for the new part to the union of the already placed part
			// take the union of this set of points to create the set of candidate positions to try
			nfp_mp_t tmp_nfp;
			nfp_mp_t nfp;
			for (polygon_t placed_part_union : union_of_placed_parts) {
				polygon_t tmp_part = part; // generateNFP() modifies it's arguments, so let it work on a copy
				tmp_nfp.clear(); 

				cout << "generateNFP(" << bg::wkt(placed_part_union) << ", " << bg::wkt(tmp_part) << ")" << endl;
				nfp_mp_t this_nfp = to_multi_point(generateNFP(placed_part_union, tmp_part, true));
				cout << "partial nfp: " << bg::wkt(this_nfp) << endl;
				bg::union_(this_nfp, nfp, tmp_nfp);
				cout << "union nfp: " << bg::wkt(tmp_nfp) << endl;
				nfp = tmp_nfp;
			}
			cout << "nfp is " << bg::wkt(nfp) << endl;

			// try all the placements given by the nfp to find the one that results 
			// in the lowest cost. For each one test that the resulting placement does not
			// overlap with any already placed parts and stays inside the bin.
			multi_polygon_t placed_parts_attempt = placed_parts;
			double best_attempt_cost = numeric_limits<double>::infinity();
			polygon_t best_placed_part_attempt;
			point_t best_position;
			bool placed = false;

			cout << "union of placed parts is " << bg::wkt(union_of_placed_parts) << endl;
			for (point_t attempt_position : nfp) {
				polygon_t placed_part_attempt = move_part_to_point(part, attempt_position);
				cout << "attempt " << attempt_position << " placed: " << bg::wkt(placed_part_attempt) << endl;

				// does it stay inside the bin?
				if (!bg::covered_by(placed_part_attempt, bin)) {
					// nope
					cout << "outside the bin" << endl;
					continue;
				}

				// does it collide with any already placed parts?
				if (bg::overlaps(union_of_placed_parts, placed_part_attempt)) {
					// no good, the new placement would overlaps with the already placed parts
					cout << "overlaps with other parts" << endl;
					continue;
				}
				else
					cout << "does not overlap" << endl;

				placed_parts_attempt.push_back(placed_part_attempt);

				cout << "union area: " << bg::area(placed_parts_attempt) << endl;

				double attempt_cost = calculate_cost(placed_parts_attempt);

				writeDebugSVG("ok-placement", placed_parts_attempt);

				if (attempt_cost < best_attempt_cost) {
					best_attempt_cost = attempt_cost;
					best_placed_part_attempt = placed_part_attempt;
					best_position = attempt_position;

					placed = true;
				}

				// clean up
				placed_parts_attempt.pop_back();
			}
			cost = best_attempt_cost;

			if (placed) {
				placed_parts.push_back(best_placed_part_attempt);

				multi_polygon_t new_union_of_placed_parts;
				bg::union_(union_of_placed_parts, best_placed_part_attempt, new_union_of_placed_parts);
				union_of_placed_parts = new_union_of_placed_parts;
				cout << "placed at " << bg::wkt(best_position) << endl;
			} else {
				// unplaceable
				cout << "unplaceable" << endl;
				unplaceable_parts.push_back(part);
			}
		}

		write_svg("final.svg", placed_parts);

		return cost;
	}

	void writeSVG(point_t p, point_t r) {
		string name = Catch::getResultCapture().getCurrentTestName();
		std::ofstream svg(name+".svg");

		polygon_t x_axis{ 
			{ { -10.0, 0.0 },{ 10.0, 0.0 } },
			{ } 
		};

		polygon_t y_axis{
			{ { 0.0, -10.0 },{ 0.0, 10.0 } }
		};

		boost::geometry::svg_mapper<point_t> mapper(svg, 400, 400);
		mapper.add(x_axis);
		mapper.add(y_axis);
		mapper.add(p);
		mapper.add(r);

		mapper.map(p, "fill-opacity:0.5;fill:rgb(153,204,0);stroke:rgb(153,204,0);stroke-width:2");
		mapper.map(r, "opacity:0.8;fill:none;stroke:rgb(255,128,0);stroke-width:4;");
		mapper.map(x_axis, "stroke:rgb(0,0,0);stroke-width:1");
		mapper.map(y_axis, "stroke:rgb(0,0,0);stroke-width:1");
	}
  
	TEST_CASE("make_transformation", "[transformation]") {
		point_t p(2.0, 3.0);
		point_t r;

		SECTION("shift up") {
			transformer_t transformer = makeTransformation(0.0, 0.0, 4.0);
			transformer.apply(p, r);
            REQUIRE(str(r) == "POINT(2.0 7.0)");
		}

		SECTION("shift right") {
			transformer_t transformer = makeTransformation( 0.0, 4.0, 0.0);
			transformer.apply(p, r);
            REQUIRE(str(r) == "POINT(6.0 3.0)");                        
		}

		SECTION("shift right and up") {
			transformer_t transformer = makeTransformation(0.0, 5.0, 7.0);
			transformer.apply(p, r);
            REQUIRE(str(r) == "POINT(7.0 10.0)");
		}

		SECTION("rotate 90deg CCW") {
			transformer_t transformer = makeTransformation(-90.0, 0.0, 0.0);
			transformer.apply(p, r);
            REQUIRE(str(r) == "POINT(-3.0 2.0)");
		}

        SECTION("rotate 90deg CCW then shift up") {
			transformer_t transformer = makeTransformation(-90.0, 0.0, 1.0);
			transformer.apply(p, r);
			REQUIRE(str(r) == "POINT(-3.0 3.0)");
		}
		
        SECTION("rotate 90deg CCW then shift right") {
			transformer_t transformer = makeTransformation(-90.0, 10.0, 0.0);
			transformer.apply(p, r);
            REQUIRE(str(r) == "POINT(7.0 2.0)");
		}

        SECTION("rotate 90deg CCW then shift to origo") {
			transformer_t transformer = makeTransformation(-90.0, 3.0, -2.0);
			transformer.apply(p, r);
            REQUIRE(str(r) == "POINT(0.0 0.0)");
		}

		SECTION("rotate 90deg CW") {
			transformer_t transformer = makeTransformation(90.0, 0.0, 0.0);
			transformer.apply(p, r);
			REQUIRE(str(r) == "POINT(3.0 -2.0)");
		}

		SECTION("rotate 90deg CW then shift to origo") {
			transformer_t transformer = makeTransformation(90.0, -3.0, 2.0);
			transformer.apply(p, r);
			REQUIRE(str(r) == "POINT(0.0 0.0)");
		}
	}

	TEST_CASE("place", "[placing]") {
		
		SECTION("inside nfp") {

			polygon_t bin{ { { 0.0, 0.0 },{ 0.0, 5.0 },{ 5.0, 5.0 },{ 5.0, 0.0 },{ 0.0, 0.0 } }}; // outer ring only
			bg::correct(bin);

			multi_polygon_t parts;
			for (double scale = 1.0; scale >= 0.1 ; scale -= 0.1) {
				polygon_t part{ { { 0.0, 0.0 },{ 0.0, scale },{ scale, scale },{ scale, 0.0 },{ 0.0, 0.0 } } };// outer ring only
				bg::correct(part);
				cout << "scale=" << scale << " part=" << bg::wkt(part) << endl;
				parts.push_back(part);
			}


			multi_polygon_t placed_parts;
			multi_polygon_t unplaceable_parts;
			double cost = place(bin, parts, placed_parts, unplaceable_parts);

			cout << "cost=" << cost << " placed_parts=" << placed_parts.size() << " unplaceable_parts=" << unplaceable_parts.size() << endl;

			placed_parts.push_back(bin);



			write_svg("result.svg", placed_parts);


			REQUIRE(true == true);
		}

		
	}
}
