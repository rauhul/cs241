/**
 * Luscious Locks Lab
 * CS 241 - Fall 2016
 */
#include "barrier.h"

// The returns are just for errors if you want to check for them.
int barrier_init(barrier_t *barrier, unsigned int num_threads) {
    barrier->n_threads = num_threads;
    barrier->count = 0;
    barrier->times_used = 1;
    pthread_mutex_init(&(barrier->mtx), NULL);
    pthread_cond_init(&(barrier->cv), NULL);
    return 0;
}

int barrier_destroy(barrier_t *barrier) {
    pthread_cond_destroy(&(barrier->cv));
    pthread_mutex_destroy(&(barrier->mtx));
    return 0;
}

int barrier_wait(barrier_t *barrier) {
    pthread_mutex_lock(&(barrier->mtx));
    // wait for barrier to active before doing shit
    while (barrier->times_used != 1) {
        pthread_cond_wait(&(barrier->cv), &(barrier->mtx));
    }
    //barrier is active
    barrier->count += 1;
    if (barrier->count == barrier->n_threads) {
        // deactivate the barrier
        barrier->times_used = 0;
        // let the other threads know
        barrier->count -= 1;
        pthread_cond_broadcast(&(barrier->cv));
    } else {
        // wait while n threads havent hit the barrier and its active
        while (barrier->count != barrier->n_threads && barrier->times_used == 1) {
            pthread_cond_wait(&(barrier->cv), &(barrier->mtx));
        }
        barrier->count -= 1;
        // if you're the last thread to leave the barrier
        if (barrier->count == 0) {
            // reactivate the barrier
            barrier->times_used = 1;
        }
        // let the other threads know
        pthread_cond_broadcast(&(barrier->cv));
    }
    pthread_mutex_unlock(&(barrier->mtx));
    return 0;
}