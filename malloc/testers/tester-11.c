/**
 * Machine Problem: Malloc
 * CS 241 - Fall 2016
 */

#include <stdio.h>
#include <stdlib.h>

#define M (1024L * 1024L)
#define SIZE (2L * 1024L * M)
#define ITERS 10000

// Check if two regions overlap
int overlap(void *r1, size_t len1, void *r2, size_t len2) {
  return ((size_t)r1 <= (size_t)r2 && (size_t)r2 < (size_t)(r1 + len1)) ||
         ((size_t)r2 <= (size_t)r1 && (size_t)r1 < (size_t)(r2 + len2));
}

// Ensure that the start and end of region are writable
void verify_write(char *ptr, size_t len) {
  *(ptr) = 'e';
  *(ptr + len - 1) = 'k';
}

int verify_read(char *ptr, size_t len) {
  int ret = 1;

  if (*ptr != 'e')
    ret = 0;

  if (*(ptr + len - 1) != 'k')
    ret = 0;

  if (ret == 0)
    fprintf(stderr, "Failure to verify data.\n");

  return ret;
}

int main() {
  malloc(1);
  int i;
  for (i = 0; i < ITERS; i++) {
    // Write to end
    char *a = malloc(SIZE);
    if (!a)
      return 1;

    verify_write(a, SIZE);
    if (!verify_read(a, SIZE))
      return 1;

    free(a);

    char *b = malloc(SIZE / 2);
    verify_write(b, SIZE / 2);

    char *c = malloc(SIZE / 4);
    verify_write(c, SIZE / 4);

    if (!b || !c)
      return 1;

    if (!verify_read(b, SIZE / 2) || !verify_read(c, SIZE / 4) ||
        overlap(b, SIZE / 2, c, SIZE / 4))
      return 1;

    free(b);
    free(c);
  }

  printf("Memory was allocated, used, and freed!\n");
  return 0;
}
