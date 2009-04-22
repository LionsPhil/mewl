#include "platform.hpp"
#include "util.hpp"

static const double ROOT_2 = 1.41421356237309504880168872420969807856967187537694807317667973799;

static double phi(double x, double stddev) {
	return 0.5 * (1 + platform_erf(x / (stddev * ROOT_2)));
}

int random_normal(int min, int max, double stddev) {
	double r = platform_random();
	for(int bound = min; bound < max; bound++) {
		if(r < phi(bound + 0.5, stddev))
			{ return bound; }
	}
	return max;
}

int random_uniform(int min, int max) {
	int r;
	do {
		r = (int) (platform_random() * ((max + 1) - min)) + min;
	} while(r > max); // 1/RAND_MAX chance of this happening
	return r;
}

