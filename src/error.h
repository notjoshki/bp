#ifndef ERROR_H
#define ERROR_H

#include <stdio.h>
#include <stdarg.h>

#define log_error(...) \
    do { \
        fprintf(stderr, "bp: Error: "); \
        fprintf(stderr, __VA_ARGS__); \
        fputc('\n', stderr); \
        increment_errors(); \
    } while (0)

#define log_dev_error(...) \
    do { \
        fprintf(stderr, "!!! bp: Dev Error: "); \
        fprintf(stderr, __VA_ARGS__); \
        fputc('\n', stderr); \
        increment_errors(); \
    } while (0)

void increment_errors();
size_t errors_logged();

#endif