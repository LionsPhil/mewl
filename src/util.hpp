#ifndef UTIL_HPP_
#define UTIL_HPP_

/** Generate a random integer in the inclusive range given with probability
 *  given by the normal distribution with mean zero and standard deviation
 *  provided. In future, this could perhaps be implemented using C++ TR1's
 *  normal_distribution malarky in <random>, but at the time of writing that
 *  appears to be somewhat on the new side of stable and well-documented. */
int random_normal(int min, int max, double stddev);

/** Generate a random number in the inclusive range given with a uniform
 *  probability distribution. Again, C++ TR1 is a future possibility here.
 *  IMPORTANT: unlike simplistic modulo-rand, max is a possible value! */
int random_uniform(int min, int max);

#endif

