/**
 * Map Reduce 0 Lab
 * CS 241 - Fall 2016
 */

#include "utils.h"

static int *descriptors_array = NULL;
static int descriptors_array_size;
static int descriptors_array_capacity;

void print_usage() {
  printf("./mr0 input_file output_file mapper_exec reducer_exec\n");
}

void descriptors_add(int fd) {
  if (!descriptors_array) {
    descriptors_array_capacity = 10;
    descriptors_array_size = 0;
    descriptors_array = (int *)malloc(sizeof(int) * descriptors_array_capacity);

    assert(descriptors_array);
  }

  if (descriptors_array_size == descriptors_array_capacity) {
    descriptors_array_capacity *= 2;
    descriptors_array = (int *)realloc(
        descriptors_array, sizeof(int) * descriptors_array_capacity);
  }

  descriptors_array[descriptors_array_size++] = fd;
}

void descriptors_closeall() {
  int i;
  for (i = 0; i < descriptors_array_size; i++) {
    close(descriptors_array[i]);
  }
}

void descriptors_destroy() {
  free(descriptors_array);
  descriptors_array = NULL;
}

void print_num_lines(char *filename) {
  int c;
  size_t count = 0;
  FILE *inf = fopen(filename, "rt");
  if (inf)
    while (EOF != (c = fgetc(inf)))
      if (c == '\n')
        count++;
  fclose(inf);
  printf("output pairs in %s: %zu\n", filename, count);
}
