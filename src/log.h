//
// Created by Joseph Hurdle on 7/13/20.
//

#ifndef SEARCHFILEC_LOG_H
#define SEARCHFILEC_LOG_H

#include <stdarg.h>

// log_message an error or message, accepts a string[str] and printf style
// varatic arguments
// [str] - string to use for log_message message
// [..arguments...] - printf style varatic arguments to format message

#define log_message(...) log_it(__FILE__, __LINE__, __VA_ARGS__)

void log_it(const char *file, int line, const char *fmt, ...);

#endif //SEARCHFILEC_LOG_H
