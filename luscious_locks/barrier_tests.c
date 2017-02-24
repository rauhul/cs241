/**
 * Luscious Locks Lab
 * CS 241 - Fall 2016
 */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "barrier.h"

#define NUM_THREADS 5

pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_t pthreads[NUM_THREADS];
barrier_t barrier;

long int count = 0;
int pass1, pass2;

void *thread_func_throwaway(void *var) {
  sleep((long int)var);
  pthread_mutex_lock(&mtx);
  count++;
  pthread_mutex_unlock(&mtx);
  barrier_wait(&barrier);

  return (void *)count;
}

void *thread_func_reusable(void *var) {
  usleep((long int)var);
  int i;
  for (i = 0; i < 3; i++) {
    pthread_mutex_lock(&mtx);
    count++;
    pthread_mutex_unlock(&mtx);
    usleep((long int)var);
    barrier_wait(&barrier);
    pthread_mutex_lock(&mtx);
    printf("%ld\n", count);
    if (count % 5)
      pass2 = 0;
    pthread_mutex_unlock(&mtx);
    barrier_wait(&barrier);
  }

  return (void *)count;
}

int main() {
  long int i, ret;
  pass1 = 1;
  pass2 = 1;

  pthread_mutex_init(&mtx, NULL);
  barrier_init(&barrier, NUM_THREADS);

  for (i = 0; i < NUM_THREADS; i++) {
    pthread_create(&pthreads[i], NULL, &thread_func_throwaway, (void *)i);
  }
  printf("Throwaway Test:\n");
  for (i = 0; i < NUM_THREADS; i++) {
    pthread_join(pthreads[i], (void **)(&ret));
    printf("%ld\n", ret);
    if (ret != 5)
      pass1 = 0;
  }

  printf("Reusable Test:\n");

  int error;
  for (i = 0; i < NUM_THREADS; i++) {
    error =
        pthread_create(&pthreads[i], NULL, &thread_func_reusable, (void *)i);
    if (error) {
      fprintf(stderr, "pthread_create failed with error %d\n", error);
      exit(1);
    }
  }

  for (i = 0; i < NUM_THREADS; i++) {
    error = pthread_join(pthreads[i], (void **)(&ret));
    if (error) {
      fprintf(stderr, "pthread_join failed with error %d\n", error);
      exit(1);
    }
  }

  if (!pass1)
    printf("Test 1 failed\n");
  else
    printf("Test 1 passed\n");
  if (!pass2)
    printf("Test 2 failed\n");
  else
    printf("Test 2 passed\n");

  pthread_mutex_destroy(&mtx);
  barrier_destroy(&barrier);

  return 0;
}
