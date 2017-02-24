/**
 * MapReduce
 * CS 241 - Fall 2016
 */

#include <stdio.h>
#include <stdlib.h>

#include "reducer.h"

const char *reducer(const char *value1, const char *value2) {
  int count1 = atoi(value1);
  int count2 = atoi(value2);

  char *res;
  asprintf(&res, "%d", count1 + count2);
  return res;
}

MAKE_REDUCER_MAIN(reducer);
