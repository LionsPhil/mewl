#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include "platform.hpp"

#ifndef RANDOM_MAX
# define RANDOM_MAX RAND_MAX
#endif

void warn(const char* fmt, ...) {
	va_list marker;
	va_start(marker, fmt);
	vfprintf(stderr, fmt, marker);
	fputs("\n", stderr);
	va_end(marker);
}

void die() { exit(EXIT_FAILURE); }

void platform_init() {
	srandom(time(0));
}

double platform_random() {
	return ((double) random()) / RANDOM_MAX;
}

double platform_erf(double x) {
	/* We shall somewhat dubiously assume here that 'POSIX' also means
	 * 'compiling with GCC (so does C99)' or 'smells like BSD'. */
	return erf(x);
}

