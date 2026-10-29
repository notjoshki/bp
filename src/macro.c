#include "macro.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// TODO: Windows support.
//#ifdef __linux__
#include <unistd.h>
//#endif

char *macro_find(const char *path) {
    char *command = malloc(strlen(path) + 11);
    sprintf(command, "$(find %s)", path);
    return command;
}