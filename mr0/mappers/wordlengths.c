/**
 * Map Reduce 0 Lab
 * CS 241 - Fall 2016
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "mapper.h"

void mapper(const char *data, FILE *output) {
  char *data_copy = strdup(data);
  char *newline = NULL;
  while ((newline = strchr(data_copy, '\n')) != NULL)
    *newline = ' ';
  char *datum = strtok(data_copy, " ");
  while (datum) {
    fprintf(output, "%zu: 1\n", strlen(datum));
    datum = strtok(NULL, " ");
  }

  free(data_copy);
}

MAKE_MAPPER_MAIN(mapper)
