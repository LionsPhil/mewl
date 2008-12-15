#include <stdlib.h>
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

