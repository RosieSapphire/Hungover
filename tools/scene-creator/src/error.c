#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "error.h"

void error_log(const char *error_format, ...)
{
#ifdef NDEBUG
	return;
#endif
	va_list args;
	va_start(args, error_format);
	fprintf(stderr, "ERROR: ");
	vfprintf(stderr, error_format, args);
	va_end(args);
	exit(EXIT_FAILURE);
}

void error_check(const int function_return, const int desired_return,
		 const char *error_message)
{
#ifdef NDEBUG
	return;
#endif
	if (function_return != desired_return)
		error_log(error_message);
}
