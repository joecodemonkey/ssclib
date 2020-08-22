//
// Created by Joseph Hurdle on 7/23/20.
//
#include "log.h"
#include <stdio.h>

void log_it(const char *file, int line, const char *fmt, ...) {
    va_list args;
    fprintf(stderr, "%s:%d [", file, line);
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "]\n");
    fflush(stderr);
}