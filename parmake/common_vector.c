/**
 * Parallel Make
 * CS 241 - Fall 2016
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "common_vector.h"

void *copy_pointer(void *rule_t) { return rule_t; }

void destroy_pointer() { return; }

void *copy_string(void *arg) {
  assert(arg);
  char *string = arg;
  return strdup(string);
}

void destroy_string(void *arg) { free(arg); }
