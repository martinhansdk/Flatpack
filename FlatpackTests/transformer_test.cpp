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
    s << bg::wkt(p);

    return s.str();
  }

  
	TEST_CASE("make_transformation", "[transformation]") {
		point_t p(2.0, 3.0);
		point_t r;

		SECTION("shift up") {
			transformer_t transformer = makeTransformation(0.0, 0.0, 0.0, 0.0, 4.0);

			transformer.apply(p, r);
                        
                        REQUIRE(str(r) == "POINT(2 7)");
		}

		SECTION("shift right") {
			transformer_t transformer = makeTransformation(0.0, 0.0, 0.0, 4.0, 0.0);

			transformer.apply(p, r);

                        REQUIRE(str(r) == "POINT(6 3)");                        
		}

		SECTION("shift right and up") {
			transformer_t transformer = makeTransformation(0.0, 0.0, 0.0, 5.0, 7.0);

			transformer.apply(p, r);

                        REQUIRE(str(r) == "POINT(7 10)");
		}

		SECTION("rotate 90deg CCW") {
			transformer_t transformer = makeTransformation(0.0, 0.0, -90.0, 0.0, 0.0);

			transformer.apply(p, r);

                        REQUIRE(str(r) == "POINT(-3 2)");
		}

                SECTION("rotate 90deg CCW then shift up") {
			transformer_t transformer = makeTransformation(0.0, 0.0, -90.0, 0.0, 1.0);

			transformer.apply(p, r);

                        REQUIRE(str(r) == "POINT(-3 3)");
		}

                SECTION("rotate 90deg CCW then shift right") {
			transformer_t transformer = makeTransformation(0.0, 0.0, -90.0, 10.0, 0.0);

			transformer.apply(p, r);

                        REQUIRE(str(r) == "POINT(7 3)");
		}

                SECTION("rotate 90deg CCW then shift to origo") {
			transformer_t transformer = makeTransformation(0.0, 0.0, -90.0, 3.0, -2.0);

			transformer.apply(p, r);

                        REQUIRE(str(r) == "POINT(0 0)");
		}
	}
}
