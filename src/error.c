#include "error.h"
#include <stdio.h>

static size_t error_count = 0;

void increment_errors() {
    error_count++;
}

size_t errors_logged() {
    return error_count;
}