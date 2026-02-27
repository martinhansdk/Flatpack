#include <sstream>
#include <iomanip>

#include "catch.hpp"
#include "../Nester/Nester.hpp"

using namespace nester;
using namespace std;

namespace NesterTests
{
	string str(point_t p) {
		stringstream s;
		s << std::setprecision(1) << std::fixed << "POINT(" << p.x << " " << p.y << ")";
		return s.str();
	}

	// SVG writing removed - was only using Boost.Geometry for visualization
   
	TEST_CASE("make_transformation", "[transformation]") {
		point_t p(2.0, 3.0);
		glm::dvec3 p3(p, 1.0);

		SECTION("shift up") {
			transformer_t transformer = makeTransformation(0.0, 0.0, 4.0);
			glm::dvec3 r3 = transformer * p3;
			point_t r(r3.x, r3.y);
            REQUIRE(str(r) == "POINT(2.0 7.0)");
		}

		SECTION("shift right") {
			transformer_t transformer = makeTransformation( 0.0, 4.0, 0.0);
			glm::dvec3 r3 = transformer * p3;
			point_t r(r3.x, r3.y);
            REQUIRE(str(r) == "POINT(6.0 3.0)");                        
		}

		SECTION("shift right and up") {
			transformer_t transformer = makeTransformation(0.0, 5.0, 7.0);
			glm::dvec3 r3 = transformer * p3;
			point_t r(r3.x, r3.y);
            REQUIRE(str(r) == "POINT(7.0 10.0)");
		}

		SECTION("rotate 90deg CCW") {
			transformer_t transformer = makeTransformation(-90.0, 0.0, 0.0);
			glm::dvec3 r3 = transformer * p3;
			point_t r(r3.x, r3.y);
            REQUIRE(str(r) == "POINT(-3.0 2.0)");
		}

        SECTION("rotate 90deg CCW then shift up") {
			transformer_t transformer = makeTransformation(-90.0, 0.0, 1.0);
			glm::dvec3 r3 = transformer * p3;
			point_t r(r3.x, r3.y);
			REQUIRE(str(r) == "POINT(-3.0 3.0)");
		}
		
        SECTION("rotate 90deg CCW then shift right") {
			transformer_t transformer = makeTransformation(-90.0, 10.0, 0.0);
			glm::dvec3 r3 = transformer * p3;
			point_t r(r3.x, r3.y);
            REQUIRE(str(r) == "POINT(7.0 2.0)");
		}

        SECTION("rotate 90deg CCW then shift to origo") {
			transformer_t transformer = makeTransformation(-90.0, 3.0, -2.0);
			glm::dvec3 r3 = transformer * p3;
			point_t r(r3.x, r3.y);
            REQUIRE(str(r) == "POINT(0.0 0.0)");
		}

		SECTION("rotate 90deg CW") {
			transformer_t transformer = makeTransformation(90.0, 0.0, 0.0);
			glm::dvec3 r3 = transformer * p3;
			point_t r(r3.x, r3.y);
			REQUIRE(str(r) == "POINT(3.0 -2.0)");
		}

		SECTION("rotate 90deg CW then shift to origo") {
			transformer_t transformer = makeTransformation(90.0, -3.0, 2.0);
			glm::dvec3 r3 = transformer * p3;
			point_t r(r3.x, r3.y);
			REQUIRE(str(r) == "POINT(0.0 0.0)");
		}
	}
}
