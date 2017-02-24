/**
 * MapReduce
 * CS 241 - Fall 2016
 */

#pragma once

#include <stdio.h>

/**
 * type which defines a mapper function.
 * Mapper functions must take a input data string, then write their output to
 * the specified FILE*
 */
typedef void (*mapper_function)(const char *, FILE *);

/**
 * runs a mapper function, reading input from the input FILE* and writing output
 * to the output FILE*
 */
int run_mapper_on_fds(FILE *input, FILE *output, mapper_function func);

/**
 * macro to create a main method which only runs the mapper function.
 */
#define MAKE_MAPPER_MAIN(func)                                                 \
  int main() { return run_mapper_on_fds(stdin, stdout, func); }
