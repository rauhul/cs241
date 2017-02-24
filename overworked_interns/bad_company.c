/**
 * Overworked Interns Lab
 * CS 241 - Fall 2016
 */
#include "company.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Bad solution that will deadlock
void *work_interns(void *p) {

  Company *company = (Company *) p;
  pthread_mutex_t *left_intern, *right_intern;

  while (running) {
    left_intern  = Company_get_left_intern(company);
    right_intern = Company_get_right_intern(company);

    // Every company grabs their left intern
    pthread_mutex_lock(left_intern);

    // Now what if another company takes the right intern
    // and won't let go?
    // Then the comppany tries to grab their right intern ...
    // DREADLOCK!
    pthread_mutex_lock(right_intern);
    Company_hire_interns(company);
    pthread_mutex_unlock(right_intern);
    pthread_mutex_unlock(left_intern);
  }
  return NULL;
}