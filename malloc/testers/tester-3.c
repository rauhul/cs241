/**
 * Machine Problem: Malloc
 * CS 241 - Fall 2016
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TOTAL_ALLOCS 6000
#define ALLOC_SIZE 1024 * 1024

int main() {
  malloc(1);

  int i;
  void *ptr = NULL;

  for (i = 0; i < TOTAL_ALLOCS; i++) {
    ptr = malloc(ALLOC_SIZE);
    if (ptr == NULL) {
      printf("Memory failed to allocate!\n");
      return 1;
    }

    memset(ptr, 0xab, ALLOC_SIZE);

    free(ptr);
  }

  printf("Memory was allocated and freed!\n");
  return 0;
}
