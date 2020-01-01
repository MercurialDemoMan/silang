#include "Error.hpp"

#include <cstdarg>
#include <cstdio>

Error::Error(Error::Type t, const char* msg, ...)
{
	this->type = t;

	std::printf("error: ");

	va_list args;

	va_start(args, msg);
	std::vprintf(msg, args);
	va_end(args);

	std::printf("\n");
}