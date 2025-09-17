#include <cstdio>

/* Error callback, outputs error to stl error stream */
static void error_callback(int error, const char* description)
{
    fputs(description, stderr);
}