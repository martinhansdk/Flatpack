#ifndef _GA_H_
#define _GA_H_

// work around compilation issues with genetic.hpp
typedef unsigned int uint;
#include "clock.hpp"

#include <algorithm>
#include <string>
#include <fstream>

#include "../libnfporb/libnfp.hpp"
#include "../openGA/src/genetic.hpp"

using namespace std;
using namespace libnfp;
using namespace std;

const double ROTATIONS[] = { 0.0, 90.0, 180.0, 270.0 };
typedef trans::rotate_transformer<bg::degree, LongDouble, 2, 2> rotate_transformer_t;
const rotate_transformer_t ROTATION_TRANSFORMS[] = {
	rotate_transformer_t(ROTATIONS[0]),
	rotate_transformer_t(ROTATIONS[1]),
	rotate_transformer_t(ROTATIONS[2]),
	rotate_transformer_t(ROTATIONS[3])
};

struct RotatedPolygon
{
	polygon_t polygon;
	int rotation;

	double outer_area() {
		//return toLongDouble(bg::area(polygon).outer);
		return 0.0; // FIXME!
	}

	polygon_t to_polygon() const {
		polygon_t rotated;
		//ROTATION_TRANSFORMS[rotation].apply(polygon, rotated); // FIXME!
		return rotated;
	}
};

struct Gene
{
	vector<RotatedPolygon> polygons;

	std::string to_string() const
	{
		stringstream ss;

		for (RotatedPolygon p : polygons) {
			ss << boost::geometry::wkt(p.polygon) << " @ " << p.rotation << endl;
		}

		return ss.str();
	}

	size_t size() const {
		return polygons.size();
	}

	vector<polygon_t> to_polygons() const {
		vector<polygon_t> rotated(polygons.size());

		for (int i = 0; i < polygons.size(); i++)
			rotated[i] = polygons[i].to_polygon();

		return rotated;
	}
};



struct MyMiddleCost
{
	// This is where the results of simulation
	// is stored but not yet finalized.
	double cost_distance2;
	double cost_sqsin;
};

typedef EA::Genetic<Gene, MyMiddleCost> GA_Type;
typedef EA::GenerationType<Gene, MyMiddleCost> Generation_Type;

bool byOuterArea(RotatedPolygon a, RotatedPolygon b) { return a.outer_area() > b.outer_area(); }

void init_genes(Gene& p, const std::function<double(void)> &rand)
{
	// initially sort by area
	std::sort(p.polygons.begin(), p.polygons.end(), byOuterArea);

	// apply random rotation
	std::random_device rd;  //Will be used to obtain a seed for the random number engine
	std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
	std::uniform_int_distribution<> dis(1, 3);

	for (RotatedPolygon rp : p.polygons) {
		rp.rotation = dis(gen);
	}
}

/*

bool eval_genes(
	const Gene& p,
	MyMiddleCost &c)
{
	double x = p.x;
	double y = p.y;
	// see the surface plot at:
	// https://academo.org/demos/3d-surface-plotter/?expression=x*x%2By*y%2B30.0*sin(x*100.0*sin(y)%2By*100.0*cos(x))%2B125%2B45.0*sqrt(x%2By)*sin((15.0*(x%2By))%2F(x*x%2By*y))&xRange=-10%2C%2B10&yRange=-10%2C%2B10&resolution=100
	// 
	// the middle comupations of cost:
	if (x + y>0)
	{
		double predictable_noise = 30.0*sin(x*100.0*sin(y) + y * 100.0*cos(x));
		c.cost_distance2 = x * x + y * y + predictable_noise;
		c.cost_sqsin = 125 + 45.0*sqrt(x + y)*sin((15.0*(x + y)) / (x*x + y * y));
		return true; // genes are accepted
	}
	else
		return false; // genes are rejected
}
/*
Gene mutate(
	const Gene& X_base,
	const std::function<double(void)> &rand,
	double shrink_scale)
{
	Gene X_new;
	double r = rand();
	bool in_range_x, in_range_y;
	double loca_scale = shrink_scale;
	if (rand()<0.4)
		loca_scale *= loca_scale;
	else if (rand()<0.1)
		loca_scale = 1.0;
	do {
		X_new = X_base;
		X_new.x += 0.2*(rand() - rand())*loca_scale;
		X_new.y += 0.2*(rand() - rand())*loca_scale;
		in_range_x = (X_new.x >= -10.0 && X_new.x<10.0);
		in_range_y = (X_new.y >= -10.0 && X_new.y<10.0);
	} while (!in_range_x || !in_range_y);
	return X_new;
}
*/
Gene crossover(
	const Gene& X1,
	const Gene& X2,
	const std::function<double(void)> &rand)
{
	std::random_device rd;  //Will be used to obtain a seed for the random number engine
	std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
	std::uniform_int_distribution<> dis(0, X1.size()-1);
	Gene X_new;
	int cutpoint = dis(gen);

	for (size_t i = 0; i <= cutpoint; i++) {
		X_new.polygons[i] = X1.polygons[i];
	}

	for (size_t i = cutpoint; i < X1.size(); i++) {
		X_new.polygons[i] = X2.polygons[i];
	}

	return X_new;
}
/*
double calculate_SO_total_fitness(const GA_Type::thisChromosomeType &X)
{
	// finalize the cost
	double cost1, cost2;
	cost1 = X.middle_costs.cost_distance2;
	cost2 = X.middle_costs.cost_sqsin;
	return cost1 + cost2;
}

std::ofstream output_file;

void SO_report_generation(
	int generation_number,
	const EA::GenerationType<Gene, MyMiddleCost> &last_generation,
	const Gene& best_genes)
{
	std::cout
		<< "Generation [" << generation_number << "], "
		<< "Best=" << last_generation.best_total_cost << ", "
		<< "Average=" << last_generation.average_cost << ", "
		<< "Best genes=(" << best_genes.to_string() << ")" << ", "
		<< "Exe_time=" << last_generation.exe_time
		<< std::endl;

	output_file
		<< generation_number << "\t"
		<< best_genes.x << "\t"
		<< best_genes.y << "\t"
		<< last_generation.average_cost << "\t"
		<< last_generation.best_total_cost << "\n";
}

int main()
{
	output_file.open("./bin/result_so1.txt");
	output_file << "step" << "\t" << "x_best" << "\t" << "y_best" << "\t" << "cost_avg" << "\t" << "cost_best" << "\n";

	EA::Chronometer timer;
	timer.tic();

	GA_Type ga_obj;
	ga_obj.problem_mode = EA::GA_MODE::SOGA;
	ga_obj.multi_threading = true;
	ga_obj.idle_delay_us = 1; // switch between threads quickly
	ga_obj.verbose = false;
	ga_obj.population = 20;
	ga_obj.generation_max = 1000;
	ga_obj.calculate_SO_total_fitness = calculate_SO_total_fitness;
	ga_obj.init_genes = init_genes;
	ga_obj.eval_genes = eval_genes;
	ga_obj.mutate = mutate;
	ga_obj.crossover = crossover;
	ga_obj.SO_report_generation = SO_report_generation;
	ga_obj.best_stall_max = 10;
	ga_obj.elite_count = 10;
	ga_obj.crossover_fraction = 0.7;
	ga_obj.mutation_rate = 0.4;
	ga_obj.solve();

	std::cout << "The problem is optimized in " << timer.toc() << " seconds." << std::endl;

	output_file.close();
	return 0;
}
*/
#endif