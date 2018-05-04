#include <sstream>

#include "catch.hpp"
//#include "../Nester/GA.hpp"
#include "../Nester/Nester.hpp"
#include <boost/geometry/io/wkt/write.hpp>

using namespace libnfp;
using namespace std;

/*

const double BIN_COST = 1.0;

typedef bg::model::multi_polygon<polygon_t> nfp_mp_t;

nfp_mp_t to_multi_polygon(const nfp_t& nfp) {
	nfp_mp_t mp;
	for (polygon_t::ring_type ring : nfp) {
		polygon_t poly;
		bg::assign(poly, ring);
		mp.push_back(poly);
	}
	return mp;
}

vector<polygon_t> place(polygon_t bin, vector<polygon_t> parts) {
	vector<polygon_t> allplacements;
	nfp_t nfp;
	  
	double cost = 1.0;

	while (parts.size() > 0) { // while there are unplaced parts
		
		vector<polygon_t> placements;
		cost += BIN_COST; // add some cost for each new bin opened

		for (polygon_t part : parts) {

			// find the nfps for the new part to all the already placed part
			// take the union of this nfp to create the resulting nfp
			nfp_mp_t tmp_nfp;
			for (polygon_t placedPart : placements) {
				tmp_nfp.clear();

				nfp_mp_t nfp = to_multi_polygon(generateNFP(placedPart, part, true));
				bg::union_(nfp, placedPart, tmp_nfp);
				nfp = tmp_nfp;
			}

			// subtract this from the bin nfp to generate the final nfp for this part
			// until we figure out how to make an inner nfp, simply use the bin as an nfp
			//nfp_mp_t bin_nfp;
			//bin_nfp.push_back(bin);

			//nfp_mp_t final_nfp;
			//bg::difference(bin_nfp, nfp, final_nfp);
		}
	}
	return allplacements;
}

*/

namespace NesterTests
{
  /*
	TEST_CASE("placer", "[placer]") {

		SECTION("inside nfp") {
			
			polygon_t bin{ { { 0.0, 0.0 },{ 0.0, 5.0 },{ 5.0, 5.0 },{ 5.0, 0.0 },{ 0.0, 0.0 } }}; // outer ring only
			bg::correct(bin);

			vector<polygon_t> parts;
			for (int i = 0; i < 10; i++) {
				polygon_t part{ { { 0.0, 0.0 },{ 0.0, 1.0 },{ 1.0, 1.0 },{ 1.0, 0.0 },{ 0.0, 0.0 } } };// outer ring only
				bg::correct(part);
				parts.push_back(part);
			}
			                

			vector<polygon_t> placed_parts = place(bin, parts);
			placed_parts.push_back(bin);

			write_svg("inside-nfp.svg", placed_parts);
			

			REQUIRE(true == true);
		}


	}

	*/
}
