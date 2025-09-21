#include <cstdio>

#include "Logger.hpp"

/* Error callback, outputs error to stl error stream */
static void error_callback(int error, const char* description)
{
    (void)std::fprintf(stderr, "Error: %d\n", error);
    LOG_INFO(description);
    // (void)fprintf(stderr, description);
}