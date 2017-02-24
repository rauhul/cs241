/**
 * Machine Problem: Password Cracker
 * CS 241 - Fall 2016
 */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "cracker1.h"
#include "thread_status.h"
#include "utils.h"

#define DEFAULT_THREADS 4

void usage() {
  fprintf(stderr, "\n  cracker1 [thread_count] < <password_file>\n\n");
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

  double start_time = getTime();
  double start_cpu_time = getCPUTime();

  // student code called here
  int ret = start(thread_count);

  // keep this timing code in the provided main.
  double elapsed = getTime() - start_time;
  double total_cpu_time = getCPUTime() - start_cpu_time;

  printf("Total time: %.2f seconds.\n", elapsed);
  printf("Total CPU time: %.2f seconds.\n", total_cpu_time);
  printf("CPU usage: %.2fx\n", total_cpu_time / elapsed);

  return ret;
}
