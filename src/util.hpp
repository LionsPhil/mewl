#ifndef UTIL_HPP_
#define UTIL_HPP_
#include <string.h>
#include <stdlib.h>

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

/** A functor that simply calls delete, for emptying vectors of pointers.
 *  Remember to clear() too. Amazing that STL doesn't contain such a beast.
 *  (Boost apparently does, as boost::lambda::delete_ptr().) */
struct delete_functor {
	template <class T> void operator() (T* p) const { delete p; }
};

/** A version of delete_functor that uses free() instead, e.g. for C strings.
 *  Valgrind can spot the difference, so we shall assume that it may matter. */
struct free_functor {
	template <class T> void operator() (T* p) const { free(p); }
};

/** A functor that compares C strings. This is because the STL-extension
 *  hash_map uses a default equal_to that compares pointers. D'oy. Be sure to
 *  provide this as the third template argument to make them actually work. */
struct hash_eqcstr {
	bool operator()(const char* one, const char* two) const;
};

#endif

