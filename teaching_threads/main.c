/**
 * Parallel Map Lab
 * CS 241 - Fall 2016
 */
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "map.h"
#include "mappers.h"
#include "par_map.h"

#define SEED 241

void validate_args(int argc, char *argv[]) {
  // Verify correct number of arguments:
  if (argc != 4) {
    fprintf(stderr, "usage: %s <mapper_name> <list_len> <num_threads>\n",
            argv[0]);
    exit(1);
  }

  // Verifying that list_len and num_threads are actually integers greater than
  // 1:
  char *endptr;
  for (int i = 2; i <= 3; i++) {
    char *str_value = argv[i];
    long value = strtol(str_value, &endptr, 10);

    if (*endptr != '\0') {
      fprintf(stderr, "Failed to convert an [%s] to a long!\n", str_value);
      exit(3);
    }

    if (value < 1) {
      fprintf(stderr, "[%s] needs to be greater than or equal to 1!\n",
              str_value);
      exit(4);
    }
  }
}

double *gen_random_list(size_t num_elems) {
  double *list = (double *)malloc(sizeof(double) * num_elems);

  for (size_t i = 0; i < num_elems; ++i) {
    list[i] = ((double)rand() / (double)RAND_MAX);
  }

  return list;
}

bool verify(double *output_list, double *input_list, mapper map_func,
            size_t list_len) {
  // call on 'map_func' to modify 'list' inplace
  // then verify element wise with 'output_list'.
  double *soln_list = map(input_list, list_len, map_func);
  for (size_t i = 0; i < list_len; ++i) {
    if (soln_list[i] != output_list[i]) {
      return false;
    }
  }
  free(output_list);
  free(soln_list);
  return true;
}

int main(int argc, char *argv[]) {
  validate_args(argc, argv);

  // Seeding random number generator
  srand(SEED);

  char *mapper_name = argv[1];
  size_t list_len = strtol(argv[2], NULL, 10);
  size_t num_threads = strtol(argv[3], NULL, 10);

  double *list = gen_random_list(list_len);
  mapper map_func = get_mapper(mapper_name);

  double *list_copy = (double *)malloc(sizeof(double) * list_len);
  memcpy(list_copy, list, sizeof(double) * list_len);

  struct timespec start, end;
  clock_gettime(CLOCK_MONOTONIC, &start);

  double *ret_list = par_map(list_copy, list_len, map_func, num_threads);

  clock_gettime(CLOCK_MONOTONIC, &end);
  double diff =
      (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1E9;
  printf("par_map ran in %f seconds\n", diff);

  bool passed = verify(ret_list, list_copy, map_func, list_len);
  if (passed) {
    printf("Congratulations you have succesfully ran par_map with %s on a list "
           "with %zu elements, and %zu threads\n",
           mapper_name, list_len, num_threads);
  }

  free(list);
  free(list_copy);
  return passed;
}
