/**
 * Luscious Locks Lab
 * CS 241 - Fall 2016
 */
#include <pthread.h>

typedef struct barrier_t {
  pthread_mutex_t mtx;
  pthread_cond_t cv;

  // Total number of threads
  unsigned int n_threads;

  // Increasing or decreasing count
  unsigned int count;

  // Keeps track of what usage number of the barrier we are at
  unsigned int times_used;
} barrier_t;

/**
 *  These functions behave near exactly like pthread_barrier's
 *  You should read the man pages for these to get a sense of how to
 *  implement them.
 */
int barrier_init(barrier_t *barrier, unsigned int num_threads);
int barrier_destroy(barrier_t *barrier);
int barrier_wait(barrier_t *barrier);