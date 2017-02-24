/**
 * Parallel Map Lab
 * CS 241 - Fall 2016
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "mappers.h"

mapper get_mapper(char *mapper_name) {
  if (strcmp(mapper_name, "triple") == 0) {
    return triple;
  } else if (strcmp(mapper_name, "negate") == 0) {
    return negate;
  } else if (strcmp(mapper_name, "slow") == 0) {
    return slow;
  }
  /* more else if clauses */
  else /* default: */ {
    fprintf(stderr, "Could not recognize [%s] as a mapper!\n", mapper_name);
    exit(4);
  }
}

double triple(double elem) { return 3 * elem; }

double negate(double elem) { return -1 * elem; }

double slow(double elem) {
  usleep(1000);
  return elem;
}
