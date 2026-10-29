#!/bin/sh
set -xe
gcc -Wall -Wextra -Wpedantic -std=c11 -march=native -s -O3 -DNDEBUG -o bp $(find src/*.c)