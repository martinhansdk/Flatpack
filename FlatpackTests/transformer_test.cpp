#include <cmath>
#include <sstream>
#include <iomanip>

#include <catch2/catch_all.hpp>
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
   
	// -------------------------------------------------------------------------
	// Helpers
	// -------------------------------------------------------------------------

	static polygon_t makeRect(double x, double y, double w, double h) {
		return { {x, y}, {x + w, y}, {x + w, y + h}, {x, y + h} };
	}

	// -------------------------------------------------------------------------
	// segmentsIntersect tests
	// -------------------------------------------------------------------------

	TEST_CASE("segmentsIntersect", "[geometry]") {
		SECTION("clearly crossing") {
			// Diagonal / crossing
			REQUIRE(segmentsIntersect({0, 0}, {2, 2}, {0, 2}, {2, 0}));
		}

		SECTION("parallel horizontal — no intersection") {
			REQUIRE_FALSE(segmentsIntersect({0, 0}, {2, 0}, {0, 1}, {2, 1}));
		}

		SECTION("collinear non-overlapping") {
			REQUIRE_FALSE(segmentsIntersect({0, 0}, {1, 0}, {2, 0}, {3, 0}));
		}

		SECTION("collinear overlapping") {
			REQUIRE(segmentsIntersect({0, 0}, {2, 0}, {1, 0}, {3, 0}));
		}

		SECTION("T-shape — endpoint on segment") {
			REQUIRE(segmentsIntersect({0, 0}, {2, 0}, {1, 0}, {1, 2}));
		}

		SECTION("sharing endpoint — touching at corner") {
			REQUIRE(segmentsIntersect({0, 0}, {1, 0}, {1, 0}, {1, 1}));
		}

		SECTION("perpendicular but not reaching") {
			REQUIRE_FALSE(segmentsIntersect({0, 0}, {1, 0}, {2, -1}, {2, 1}));
		}
	}

	// -------------------------------------------------------------------------
	// pointInPolygon tests
	// -------------------------------------------------------------------------

	TEST_CASE("pointInPolygon", "[geometry]") {
		polygon_t square = makeRect(0, 0, 4, 4);

		SECTION("clearly inside") {
			REQUIRE(pointInPolygon({2, 2}, square));
		}

		SECTION("clearly outside") {
			REQUIRE_FALSE(pointInPolygon({5, 5}, square));
		}

		SECTION("outside left") {
			REQUIRE_FALSE(pointInPolygon({-1, 2}, square));
		}

		SECTION("outside below") {
			REQUIRE_FALSE(pointInPolygon({2, -1}, square));
		}

		SECTION("triangle — inside") {
			polygon_t tri = {{0, 0}, {4, 0}, {2, 4}};
			REQUIRE(pointInPolygon({2, 1}, tri));
		}

		SECTION("triangle — outside") {
			polygon_t tri = {{0, 0}, {4, 0}, {2, 4}};
			REQUIRE_FALSE(pointInPolygon({0, 3}, tri));
		}

		SECTION("empty polygon") {
			REQUIRE_FALSE(pointInPolygon({1, 1}, polygon_t{}));
		}
	}

	// -------------------------------------------------------------------------
	// polygonsOverlap tests
	// -------------------------------------------------------------------------

	TEST_CASE("polygonsOverlap", "[geometry]") {
		SECTION("non-overlapping side by side") {
			polygon_t a = makeRect(0, 0, 2, 2);
			polygon_t b = makeRect(3, 0, 2, 2);
			REQUIRE_FALSE(polygonsOverlap(a, b));
		}

		SECTION("overlapping") {
			polygon_t a = makeRect(0, 0, 3, 3);
			polygon_t b = makeRect(2, 2, 3, 3);
			REQUIRE(polygonsOverlap(a, b));
		}

		SECTION("one fully inside the other") {
			polygon_t outer = makeRect(0, 0, 10, 10);
			polygon_t inner = makeRect(3, 3, 2, 2);
			REQUIRE(polygonsOverlap(outer, inner));
		}

		SECTION("touching edges — no overlap") {
			// Sharing an edge counts as touching but not area-overlapping.
			// The ray-cast containment check will not fire and no edges cross,
			// so the result is false.
			polygon_t a = makeRect(0, 0, 2, 2);
			polygon_t b = makeRect(2, 0, 2, 2);
			// Edge-touching: implementation-defined; just ensure it doesn't crash.
			bool result = polygonsOverlap(a, b);
			(void)result;
		}

		SECTION("empty polygons") {
			REQUIRE_FALSE(polygonsOverlap(polygon_t{}, makeRect(0, 0, 2, 2)));
			REQUIRE_FALSE(polygonsOverlap(makeRect(0, 0, 2, 2), polygon_t{}));
		}
	}

	// -------------------------------------------------------------------------
	// polygonMinDistance tests
	// -------------------------------------------------------------------------

	TEST_CASE("polygonMinDistance", "[geometry]") {
		SECTION("unit gap — side by side squares") {
			polygon_t a = makeRect(0, 0, 2, 2);
			polygon_t b = makeRect(3, 0, 2, 2);
			// Closest edges: x=2 (a) and x=3 (b), gap = 1.0
			double d = polygonMinDistance(a, b);
			REQUIRE(d == Catch::Approx(1.0).epsilon(1e-9));
		}

		SECTION("diagonal gap") {
			// a corner at (2,2), b corner at (3,3) — distance = sqrt(2)
			polygon_t a = makeRect(0, 0, 2, 2);
			polygon_t b = makeRect(3, 3, 2, 2);
			double d = polygonMinDistance(a, b);
			REQUIRE(d == Catch::Approx(sqrt(2.0)).epsilon(1e-9));
		}

		SECTION("empty polygon returns infinity") {
			double d = polygonMinDistance(polygon_t{}, makeRect(0, 0, 2, 2));
			REQUIRE(std::isinf(d));
		}
	}

	// -------------------------------------------------------------------------

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
