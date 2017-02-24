/**
 * Machine Problem: Malloc
 * CS 241 - Fall 2016
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MIN_ALLOC_SIZE (128 * 1024 * 1024)
#define MAX_ALLOC_SIZE (1024 * 1024 * 1024)

void verify(void *region, int c, size_t len) {
  char *r = region;
  while (len--)
    if (*(r++) != (char)c) {
      printf("Memeory failed to contain correct value!\n");
      exit(1);
    }
}

int overlap(void *r1, void *r2, size_t len) {
  return (size_t)r1 <= (size_t)r2 && (size_t)r2 < (size_t)(r1 + len);
}

void verify_overlap3(void *r1, void *r2, void *r3, size_t len) {
  if (overlap(r1, r2, len) || overlap(r1, r3, len) || overlap(r2, r3, len)) {
    printf("Memory regions overlap!\n");
    exit(1);
  }
}

void *malloc_and_break(void *region, int c, size_t len) {
  if (len < MIN_ALLOC_SIZE) {
    return region;
  }

  void *sr1 = realloc(region, len / 3);
  void *sr2 = malloc(len / 3);
  void *sr3 = malloc(len / 3);

  verify_overlap3(sr1, sr2, sr3, len / 3);
  verify(sr1, c, len / 3);

  memset(sr1, 0xab, len / 3);
  memset(sr2, 0xcd, len / 3);
  memset(sr3, 0xef, len / 3);
  free(sr1);
  free(sr3);

  sr2 = malloc_and_break(sr2, 0xcd, len / 3);
  sr2 = realloc(sr2, len);

  verify(sr2, 0xcd, len / 3);
  memset(sr2, c, len);

  return sr2;
}

int main() {
  malloc(1);

  size_t len = MAX_ALLOC_SIZE;
  while (len > MIN_ALLOC_SIZE) {
    void *mem = malloc(len);
    memset(mem, 0xff, len);
    free(malloc_and_break(mem, 0xff, len));
    len /= 3;
  }

  printf("Memory was allocated and freed!\n");
  return 0;
}
