#include "catch.hpp"
#include "../libnfporb/libnfp.hpp"
#include "../Nester/Nester.hpp"

using namespace nester;

namespace NesterTests
{		
	TEST_CASE("make_transformation", "[transformation]") {
		point_t p(2.0, 3.0);
		point_t r;

		SECTION("shift up") {
			transformer_t transformer = makeTransformation(0.0, 0.0, 0.0, 0.0, 4.0);

			transformer.apply(p, r);

			REQUIRE(r.x_.val() == 2.0);
			REQUIRE(r.y_.val() == 7.0);
		}

		SECTION("shift right") {
			transformer_t transformer = makeTransformation(0.0, 0.0, 0.0, 4.0, 0.0);

			transformer.apply(p, r);

			REQUIRE(r.x_.val() == 6.0);
			REQUIRE(r.y_.val() == 3.0);
		}

		SECTION("shift right and up") {
			transformer_t transformer = makeTransformation(0.0, 0.0, 0.0, 5.0, 7.0);

			transformer.apply(p, r);

			REQUIRE(r.x_.val() == 7.0);
			REQUIRE(r.y_.val() == 10.0);
		}

		SECTION("rotate 90deg CCW") {
			transformer_t transformer = makeTransformation(0.0, 0.0, -90.0, 0.0, 0.0);

			transformer.apply(p, r);

			REQUIRE(r.x_.val() == -3.0);
			REQUIRE(r.y_.val() ==  2.0);
		}
	}
}