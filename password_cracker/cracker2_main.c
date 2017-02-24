/**
 * Machine Problem: Password Cracker
 * CS 241 - Fall 2016
 */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include "cracker2.h"
#include "thread_status.h"

#define DEFAULT_THREADS 4

void usage() {
  fprintf(stderr, "\n  cracker2 [thread_count] < <password_file>\n\n");
  exit(1);
}

int main(int argc, char **argv) {
  size_t thread_count = DEFAULT_THREADS;

  if (argc > 2)
    usage();

  if (argc > 1) {
    if (1 != sscanf(argv[1], "%lu", &thread_count) || thread_count < 1)
      usage();
  }

  // capture ctrl-c
  signal(SIGINT, threadStatusPrint);

  // student code called here
  int ret = start(thread_count);

  return ret;
}
