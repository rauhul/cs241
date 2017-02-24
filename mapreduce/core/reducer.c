/**
 * MapReduce
 * CS 241 - Fall 2016
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "libds.h"
#include "reducer.h"
#include "utils.h"

static FILE *whereto = NULL;

void print_ds(const char *key, const char *value, void *arg) {
  assert(whereto);
  fprintf(whereto, "%s: %s\n", key, value);
  fflush(whereto);
}

int run_reducer_on(FILE *input, FILE *output, reducer_fun func) {

  datastore_t my_datastore;
  datastore_init(&my_datastore);

  char *line = NULL;
  size_t size = 0;

  while (getline(&line, &size, input) != -1) {
    char *key = NULL;
    char *value = NULL;

    if (!split_key_value(line, &key, &value)) {
      fprintf(stderr, "reducer input is malformed: %s\n", line);
      continue;
    }

    // datastore_get returns a copy of the string
    const char *curr_val = datastore_get(&my_datastore, key);
    if (curr_val) {
      const char *new_value = func(curr_val, value);
      datastore_update(&my_datastore, key, new_value);

      free((char *)curr_val);
      free((char *)new_value);
    } else {
      datastore_put(&my_datastore, key, value);
    }
  }

  // FIXME not thread safe, but could be. lock it or use thread local storage
  whereto = output;
  datastore_iterate(&my_datastore, print_ds, NULL);
  whereto = NULL;

  datastore_destroy(&my_datastore);
  free(line);

  return 0;
}
