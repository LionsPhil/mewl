#ifndef PLATFORM_HPP_
#define PLATFORM_HPP_

/// Print an error string to the appropriate place. Automatic newline.
void warn(const char* fmt, ...);

// Print a string to standard error or equiv. only on debug builds.
#ifdef NDEBUG
static inline void _noop(const char* fmt, ...) {}
# define trace _noop
#else
# ifdef PLATFORMposix
#  define trace warn
# else
void _trace(const char* fmt, ...);
#  define trace _trace
# endif
#endif

/// Exit with failure. warn() first if you want a message (advisable).
void die();

/// Initialise platform-specific things; e.g. seed the random number generator.
void platform_init();

/// Generate a random fractional number between 0 and 1.
double platform_random();

/// Calculate the error function (this is in C99 as erf()).
double platform_erf(double x);

#endif

