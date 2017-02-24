/**
 * Machine Problem: Password Cracker
 * CS 241 - Fall 2016
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "utils.h"

// make argc global
static int argc_global;

void *busy_work(void *args __attribute__((unused))) {
  // busy work
  int i;
  double x = argc_global < 0 ? 1.9 : 2.0;
  for (i = 0; i < 50000000; i++)
    x = (x + 6) * .25;

  // convince the compiler not to optimize out the loop
  if (x != 2.0)
    printf("Shouldn't happen.\n");

  return NULL;
}

/**
 * Demonstrate the difference between CPU and 'wall clock' time.
 */
int main(int argc, __attribute__((unused)) char **argv) {
  argc_global = argc;

  // sleep doesn't use CPU time
  double start_wall_time = getTime();
  double start_cpu_time = getCPUTime();

  sleep(1);

  double elapsed_wall = getTime() - start_wall_time;
  double elapsed_cpu = getCPUTime() - start_cpu_time;
  printf("sleep(1): %.2f seconds wall time, %.2f seconds CPU time\n",
         elapsed_wall, elapsed_cpu);

  // do some busy work for a while, the CPU time and wall clock time will be the
  // same!
  start_wall_time = getTime();
  start_cpu_time = getCPUTime();

  busy_work(NULL);

  elapsed_wall = getTime() - start_wall_time;
  elapsed_cpu = getCPUTime() - start_cpu_time;
  printf("single threaded cpu work: %.2f seconds wall time, %.2f seconds CPU "
         "time\n",
         elapsed_wall, elapsed_cpu);

  // start 4 threads to do some work. On the VMs, there should be 2 cores. This
  // means we should see:
  // 4*wall clock time ~= cpu time
  start_wall_time = getTime();
  start_cpu_time = getCPUTime();

  pthread_t threads[4];
  for (size_t i = 0; i < 4; i++) {
    int ret = pthread_create(&threads[i], NULL, busy_work, NULL);
    if (ret) {
      perror("something went wrong!");
      exit(1);
    }
  }

  for (size_t i = 0; i < 4; i++) {
    pthread_join(threads[i], NULL);
  }

  elapsed_wall = getTime() - start_wall_time;
  elapsed_cpu = getCPUTime() - start_cpu_time;
  printf("multi threaded cpu work: %.2f seconds wall time, %.2f seconds CPU "
         "time\n",
         elapsed_wall, elapsed_cpu);

  return 0;
}
