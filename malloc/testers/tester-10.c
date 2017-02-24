/**
 * Machine Problem: Malloc
 * CS 241 - Fall 2016
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define M (1024L * 1024L)
#define SIZE (128 * M)
#define ITERS 100000

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
    printf("Failure to verify data.\n");

  return ret;
}

int main() {
  malloc(1);

  int i;
  for (i = 0; i < ITERS; i++) {
    char *a = malloc(SIZE + i);
    if (!a)
      return 1;

    verify_write(a, SIZE);

    int *b = malloc(SIZE + i);
    if (!b)
      return 1;

    verify_write(a, SIZE + i);

    if (!verify_read(a, SIZE))
      return 1;
    if (!verify_read(a, SIZE + i))
      return 1;

    free(a);
    free(b);

    a = malloc(2 * (SIZE + i));
    if (!a)
      return 1;

    verify_write(a, SIZE);
    if (!verify_read(a, SIZE))
      return 1;

    free(a);
  }

  printf("Memory was allocated, used, and freed!\n");
  return 0;
}
