/**
 * Ideal Indirection Lab
 * CS 241 - Fall 2016
 */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "mmu.h"

// Struct for storing the bounds
typedef struct {
  size_t pid;
  void **virtual_addresses;
} process_t;

// Global MMU
MMU *mmu;
size_t num_processes;
size_t num_addresses;
size_t num_iterations;
size_t seed;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
// This function parses the arguments in argv.
// An error message is printed if any of the following happens:
//  * An incorrect number of arguments are passed in.
//  * An argument is not a long.
//  * An argument is less than 1.
// Else an array of 4 size_t elements are returned
// in the following order: 'num_processes', 'num_addresses', 'num_iterations',
// 'seed'.
// It is the callers responsibility to free the memory used.
size_t *arg_parse(int argc, char *argv[]) {
  if (argc != 5) {
    fprintf(
        stderr,
        "usage: %s [num_processes] [num_addresses] [num_iterations] [seed]\n",
        argv[0]);
    exit(1);
  }

  char *endptr;
  long tokargs[argc - 1];
  size_t num_tokens = argc - 1;
  for (size_t i = 0; i < num_tokens; i++) {
    tokargs[i] = strtol(argv[i + 1], &endptr, 10);
    if (*endptr != '\0') {
      fprintf(stderr, "Failed to convert an argument to a long!\n");
      exit(2);
    }
    if (tokargs[i] < 1) {
      fprintf(stderr,
              "Please have all arguments be greater than or equal to 1!\n");
      exit(3);
    }
  }
  size_t *args = malloc(num_tokens * sizeof(size_t));
  for (size_t i = 0; i < num_tokens; i++) {
    args[i] = (size_t)tokargs[i];
  }
  return args;
}

void *ask_mmu_for_physical_address(void *virtual_address, size_t pid) {
  printf("CPU is asking the MMU what is the physical_address of [%p] is for "
         "pid [%lu] ...\n",
         virtual_address, pid);
  void *physical_address = MMU_get_physical_address(mmu, virtual_address, pid);
  printf("CPU got [%p]\n\n", physical_address);
  return physical_address;
}

// Start routine for each thread
void *get_physical_addresses(void *ptr) {
  process_t *process = (process_t *)ptr;
  void **virtual_addresses = process->virtual_addresses;
  void **physical_addresses = malloc(num_addresses * sizeof(void *));
  size_t pid = process->pid;
  for (size_t i = 0; i < num_iterations; i++) {
    for (size_t j = 0; j < num_addresses; j++) {
      void *virtual_address = virtual_addresses[j];
      pthread_mutex_lock(&m);
      physical_addresses[j] =
          ask_mmu_for_physical_address(virtual_address, pid);
      pthread_mutex_unlock(&m);
      // Give another process a chance to request address :)
      usleep(250000);
    }
  }
  return physical_addresses;
}

int main(int argc, char *argv[]) {
  size_t *args = arg_parse(argc, argv);
  num_processes = args[0];
  num_addresses = args[1];
  num_iterations = args[2];
  seed = args[3];

  // Create the timespec structs
  struct timespec start_time, end_time;
  // set timer before simulation
  clock_gettime(CLOCK_MONOTONIC, &start_time);

  printf("Running the simulator (with seed: [%lu]) with [%lu] processes "
         "requesting [%lu] addresses [%lu] times each\n\n\n",
         seed, num_processes, num_addresses, num_iterations);
  // Seeding the random number generator
  srand(seed);
  mmu = MMU_create();
  process_t *processes = malloc(sizeof(process_t) * num_processes);

  // Generate random process struct (yes this can collide)
  for (size_t i = 0; i < num_processes; i++) {
    process_t *process = &processes[i];
    process->pid = rand() % MAX_PROCESS_ID;
    MMU_add_process(mmu, process->pid);
    process->virtual_addresses = malloc(sizeof(void *) * num_addresses);
    for (size_t j = 0; j < num_addresses; j++) {
      // 1 << VIRTUAL_ADDRESS_LENGTH is the same as 2 ^ (VIRTUAL_ADDRESS_LENGTH)
      // which is the max virtual address.
      char *ptr = NULL;
      process->virtual_addresses[j] =
          ptr + (rand() % ((size_t)1 << VIRTUAL_ADDRESS_LENGTH));
    }
  }

  pthread_t *threads = (pthread_t *)malloc(num_processes * sizeof(pthread_t));
  for (size_t i = 0; i < num_processes; i++) {
    pthread_create(&threads[i], NULL, get_physical_addresses, &processes[i]);
  }

  void **physical_addresses = malloc(num_processes * sizeof(void *));
  for (size_t i = 0; i < num_processes; i++) {
    pthread_join(threads[i], &physical_addresses[i]);
    free(processes[i].virtual_addresses);
    free(physical_addresses[i]);
  }

  // Stop the timer once the simulation finishes
  clock_gettime(CLOCK_MONOTONIC, &end_time);
  // Calculate the difference
  double seconds_diff = end_time.tv_sec - start_time.tv_sec;
  double nanoseconds_diff = end_time.tv_nsec - start_time.tv_nsec;
  double duration = seconds_diff + nanoseconds_diff / 1e9;

  printf("\n\nSimulation has stopped with the following statistics:\n\n");
  printf("Number of Page Faults: %lu\n", mmu->num_page_faults);
  printf("Number of TLB Cache Misses: %lu\n", mmu->num_tlb_misses);
  printf("Time Elapsed: %lf seconds\n", duration);

  MMU_delete(mmu);
  free(processes);
  free(threads);
  free(physical_addresses);
  free(args);
  return 0;
}
