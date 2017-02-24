/**
 * Overworked Interns Lab
 * CS 241 - Fall 2016
 */
#include "company.h"
#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

/**
 * This is the struct for a Company object,
 * but you should not modify it directly.
 * Think of all the member variables as being private.
 * So you must call on the public API.
 */
struct Company {
  pthread_mutex_t *left_intern, *right_intern;
  int company_number;
  pthread_t thread;
  int job_duration;
  int failed;
  int billable_days;
  int tid;
  int x, y; // Screen position
};

Company *Company_create() {
  Company *company = (Company *)calloc(1, sizeof(Company));
  return company;
}

void Company_hire_interns(Company *company) {
  usleep(rand() %
         delta); // Sleep for sometime to visualize company trying to get intern

  pthread_mutex_t *left_intern = Company_get_left_intern(company);
  pthread_mutex_t *right_intern = Company_get_right_intern(company);

  int company_thread = Company_get_tid(company);
  int left_owner = left_intern->__data.__owner;
  int right_owner = right_intern->__data.__owner;

  if (!left_owner || !right_owner) {
    printf(ANSI_COLOR_RESET
           "Both interns need to be locked before you hire them!\n");
    exit(1);
  }
  if (right_owner != left_owner) {
    printf(ANSI_COLOR_RESET "Both interns need to be locked by the SAME "
                            "company before you hire them!\n");
    exit(1);
  }
  if (company_thread != right_owner || company_thread != left_owner) {
    printf(ANSI_COLOR_RESET "The company that has locked the interns must be "
                            "the one to hire them! \n");
    exit(1);
  }
  working = 1;
  printf(ANSI_COLOR_YELLOW "\033[?25l\033[%d;%dH%d is working    \n",
         (int)Company_get_Y(company), (int)Company_get_X(company),
         Company_get_company_number(company));
  usleep(Company_get_job_duration(company));
  company->billable_days++;
  if (rand() % 2 == 0) {
    // After working a company might want to have a board meeting.
    Company_have_board_meeting(company);
  }
}

void Company_have_board_meeting(Company *company) {
  printf(ANSI_COLOR_BLUE "\033[?25l\033[%d;%dH%d is meeting    \n",
         (int)Company_get_Y(company), (int)Company_get_X(company),
         Company_get_company_number(company));
  usleep(rand() % delta);
}

pthread_mutex_t *Company_get_left_intern(Company *company) {
  return company->left_intern;
}

void Company_set_left_intern(Company *company, pthread_mutex_t *left_intern) {
  company->left_intern = left_intern;
}

pthread_mutex_t *Company_get_right_intern(Company *company) {
  return company->right_intern;
}

void Company_set_right_intern(Company *company, pthread_mutex_t *right_intern) {
  company->right_intern = right_intern;
}

pthread_t *Company_get_thread(Company *company) { return &company->thread; }

int Company_get_failed_flag(Company *company) { return company->failed; }

void Company_set_failed_flag(Company *company, int failed) {
  company->failed = failed;
}

int Company_get_company_number(Company *company) {
  return company->company_number;
}

void Company_set_company_number(Company *company, int company_number) {
  company->company_number = company_number;
}

int Company_get_job_duration(Company *company) { return company->job_duration; }

double Company_get_X(Company *company) { return company->x; }

void Company_set_X(Company *company, double x) { company->x = x; }

double Company_get_Y(Company *company) { return company->y; }

void Company_set_Y(Company *company, double y) { company->y = y; }

void Company_set_job_duration(Company *company, int job_duration) {
  company->job_duration = job_duration;
}
int Company_get_billable_days(Company *company) {
  return company->billable_days;
}
void Company_set_tid(Company *company, int tid) { company->tid = tid; }
int Company_get_tid(Company *company) { return company->tid; }
