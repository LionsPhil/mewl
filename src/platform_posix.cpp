#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include "platform.hpp"

void warn(const char* fmt, ...) {
	va_list marker;
	va_start(marker, fmt);
	vfprintf(stderr, fmt, marker);
	fputs("\n", stderr);
	va_end(marker);
}

void die() { exit(EXIT_FAILURE); }

