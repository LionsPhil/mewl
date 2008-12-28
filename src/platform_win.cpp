#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include "platform.hpp"

void warn(const char* fmt, ...) {
	char buffer[256];
	va_list marker;
	va_start(marker, fmt);
	vsnprintf(buffer, sizeof buffer, fmt, marker);
	buffer[255] = '\0';
	MessageBox(0, buffer, "M.E.W.L.", MB_OK | MB_ICONWARNING);
	va_end(marker);
}

void _trace(const char* fmt, ...) {
	char buffer[256];
	va_list marker;
	va_start(marker, fmt);
	vsnprintf(buffer, sizeof buffer, fmt, marker);
	buffer[255] = '\0';
	OutputDebugString(buffer);
	va_end(marker);
}

void die() { exit(EXIT_FAILURE); }

void platform_init() {
	srand((unsigned) time(NULL));
}

double platform_random() {
	// rand_s appears to be overkill, a la /dev/random on Unicies
	return ((double) rand()) / RAND_MAX;
}

/* Unfun: Microsoft's libraries apparently don't provide an implementation of
 * this. Possibly consider using BSD's implementation, which appears to be
 * reasonably self-contained (and licensed). Alternatively, if Boost-ing to
 * get stdint, it provides a fancy-pants templated version of erf(). */
double plat_erf(double x) { return erf(x); } // Expect this to not compile :(

