/**
 * Overworked Interns Lab
 * CS 241 - Fall 2016
 */
#include "company.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>

void *work_interns(void *p) { 

    Company *company = (Company *) p;
    pthread_mutex_t *left_intern, *right_intern;

    while (running) {
        left_intern  = Company_get_left_intern(company);
        right_intern = Company_get_right_intern(company);

        if (Company_get_company_number(company) % 2) {
            pthread_mutex_t *temp = left_intern;
            left_intern = right_intern;
            right_intern = temp;
        }

        pthread_mutex_lock(left_intern);
        pthread_mutex_lock(right_intern);
        Company_hire_interns(company);
        pthread_mutex_unlock(right_intern);
        pthread_mutex_unlock(left_intern);
        Company_have_board_meeting(company);
    }
	return NULL;
}