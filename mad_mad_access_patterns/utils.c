/**
 * Mad Mad Access Pattern
 * CS 241 - Fall 2016
 */
#include "utils.h"
#include <stdlib.h>

void openFail(const char *file_name) {
  fprintf(stderr, "Cannot open %s\n", file_name);
}

void mmapFail(const char *file_name) {
  fprintf(stderr, "Failed to map %s\n", file_name);
}

void formatFail(const char *file_name) {
  fprintf(stderr, "%s: Incorrect format\n", file_name);
}

void printFound(const char *word, const uint32_t count, const float price) {
  printf("%s: %u at $%.2f\n", word, count, price);
}

void printNotFound(const char *word) { printf("%s not found\n", word); }

void printArgumentUsage() {
  fprintf(stderr, "\nUsage: ./lookup(1/2) <data_file> <word> [<word> ...]\n\n");
}
