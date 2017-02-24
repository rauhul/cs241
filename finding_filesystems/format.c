/**
* Finding Filesystems
* CS 241 - Fall 2016
*/
#include "format.h"
#include <stdio.h>
#include <unistd.h>

int is_terminal = 10;

void print_directory(char *name) {
  if (is_terminal == 10) {
    is_terminal = isatty(1);
  }
  if (is_terminal) {
    printf("\033[1;34m%s/\e[0m\n", name);
  } else {
    printf("%s/\n", name);
  }
}

void print_file(char *name) {
  if (is_terminal == 10) {
    is_terminal = isatty(1);
  }
  if (is_terminal) {
    printf("\033[1;32m%s\e[0m\n", name);
  } else {
    printf("%s\n", name);
  }
}

void print_no_file_or_directory() {
  if (is_terminal == 10) {
    is_terminal = isatty(1);
  }
  const char *message = "No Such File Or Directory";
  if (is_terminal) {
    printf("\033[1;31m%s\e[0m\n", message);
  } else {
    printf("%s\n", message);
  }
}
