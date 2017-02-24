/**
 * MapReduce
 * CS 241 - Fall 2016
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "mapper.h"

int run_mapper_on_fds(FILE *input, FILE *output, mapper_function func) {
  char *line = NULL;
  size_t len = 0;
  ssize_t str_len;

  while (-1 != (str_len = getline(&line, &len, input))) {
    // remove newline
    if (str_len > 0 && line[str_len - 1] == '\n') {
      line[--str_len] = '\0';

      // and carriage return
      if (str_len > 0 && line[str_len - 1] == '\r') {
        line[--str_len] = '\0';
      }
    }
    func(line, output);
    fflush(output);
  }

  free(line);
  return 0;
}
