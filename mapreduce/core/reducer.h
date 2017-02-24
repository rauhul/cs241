/**
 * MapReduce
 * CS 241 - Fall 2016
 */

#pragma once

#include <stdio.h>

typedef const char *(*reducer_fun)(const char *, const char *);

/**
 * Runs the reducer function on the input FILE *, outputting to the output FILE*
 *
 * NOT THREAD SAFE.
 */
int run_reducer_on(FILE *input, FILE *output, reducer_fun func);

#define MAKE_REDUCER_MAIN(func)                                                \
  int main() { return run_reducer_on(stdin, stdout, func); }
