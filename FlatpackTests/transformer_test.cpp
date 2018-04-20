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
}
