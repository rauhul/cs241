/**
 * Overworked Interns Lab
 * CS 241 - Fall 2016
 */
#include "company.h"
#include <errno.h>
#include <math.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

volatile int working = 0; // Ugly Ugly way to poll all threads
volatile int running = 1; // Flag for keeping the simulator on (dont touch it!)
unsigned int delta =
    100000; // Time delta for operations like trying, working, and meeting.
int Signum = 0;
// Whoa! A global mutex .. I wonder what this can be used for :)
pthread_mutex_t arbitrator = PTHREAD_MUTEX_INITIALIZER;

// Global barrier to ensure companies start at the same time
pthread_barrier_t global_barrier;

int num_companies = 0;
// Wrapper function that is used for thread creation
// Blocks on global barrier and then calls the work_interns() function from
// good_company.c
// Helps with fairness
void *start_running_company(void *p) {
  Company *company = (Company *)p;
  Company_set_tid(company, syscall(SYS_gettid));
  pthread_barrier_wait(&global_barrier);
  return work_interns(company);
}

void *surveillance_start(void *cp) {
  Company **companies = (Company **)cp;
  pthread_barrier_wait(&global_barrier);

  usleep(delta * 3);

  int only_one_intern = 0, deadlock = 0;

  while (running == 1 && only_one_intern == 0) {
    int working_threads = 0;
    for (int i = 0; i < num_companies; i++) {
      Company *company = (Company *)companies[i];
      int company_thread = Company_get_tid(company);
      pthread_mutex_t *left_intern = Company_get_left_intern(company);
      pthread_mutex_t *right_intern = Company_get_right_intern(company);

      if (left_intern == right_intern) {
        only_one_intern = 1;
        break;
      }

      int left_intern_owner = left_intern->__data.__owner;
      int right_intern_owner = right_intern->__data.__owner;
      if ((company_thread == left_intern_owner &&
           company_thread != right_intern_owner) ||
          (company_thread != left_intern_owner &&
           company_thread == right_intern_owner))
        printf(ANSI_COLOR_RED "\033[?25l\033[%d;%dH%d is trying    \n",
               (int)Company_get_Y(company), (int)Company_get_X(company),
               Company_get_company_number(company));
      if (company_thread == left_intern_owner &&
          company_thread == right_intern_owner)
        working_threads++;
    }
    if ((working_threads == 0 && working == 0) || only_one_intern == 1) {
      deadlock = 1;
      break;
    }
    usleep(delta);
  }

  printf(ANSI_COLOR_RESET "\033[?25h\033[20;H");

  if (deadlock == 1)
    printf("Exiting due to potential deadlock\n");
  else
    printf("Exiting\n");
  if (deadlock == 1 || Signum == SIGINT) {
    for (int i = 0; i < num_companies; i++) {
      Company *company = (Company *)companies[i];
      pthread_t company_thread = *(Company_get_thread(company));
      if (0 != pthread_kill(company_thread, SIGTERM))
        fprintf(stderr, "pthread_kill failed\n");
    }
  }
  return NULL;
}

size_t *arg_parse(int argc, char *argv[]) {
  if (argc != 3 && argc != 4) {
    fprintf(stderr, "usage: %s [num_companies] [num_interns]\n", argv[0]);
    fprintf(stderr, "OR usage: %s [num_companies] [num_interns] [delta]\n",
            argv[0]);
    exit(1);
  }

  char *endptr;
  long tokargs[argc - 1];
  size_t num_args = sizeof(tokargs) / sizeof(tokargs[0]);
  for (size_t i = 0; i < num_args; i++) {
    tokargs[i] = strtol(argv[i + 1], &endptr, 10);
    if (*endptr != '\0') {
      fprintf(stderr, "Failed to convert an arugment to a long!\n");
      exit(2);
    }
    if (tokargs[i] < 1) {
      fprintf(stderr,
              "Please have all arguments be greater than or equal to 1!\n");
      exit(3);
    }
  }
  size_t *args = malloc(num_args * sizeof(size_t));
  for (size_t i = 0; i < num_args; i++) {
    args[i] = (size_t)tokargs[i];
  }
  return args;
}

void signalHandler(int signum) {
  Signum = signum; // DO Nothing
  running = 0;
}

// Roughly based on code from http://rosettacode.org/
int main(int argc, char *argv[]) {
  setvbuf(stdout, 0, _IONBF, 0); // no buffering
  printf("\033[H\033[J");        // Clear

  if (signal(SIGINT, signalHandler) != 0 ||
      signal(SIGALRM, signalHandler) != 0) {
    fprintf(stderr, "cannot establish signal handler\n");
    exit(1);
  }

  // Arg Parsing magic
  size_t *args = arg_parse(argc, argv);

  srand(241); // Make rand()/scheduling somewhat repeatable
  // Exercise for the reader: what if there is only one intern?
  num_companies = args[0];
  int num_interns = args[1];
  if (argc == 4 && args[2] >= delta)
    delta = args[2];

  printf(ANSI_COLOR_RESET
         "Running simulation with %d companies and %d interns\n",
         num_companies, num_interns);

  // One mutex per intern
  // Since humans are inherently a critical section.
  pthread_mutex_t interns[num_interns];
  memset(interns, 0x00, num_interns * sizeof(pthread_mutex_t));
  Company *companies[num_companies];

  // Creating the companies
  for (int i = 0; i < num_companies; ++i) {
    companies[i] = Company_create();
  }

  // Initializing said mutexes
  for (int i = 0; i < num_interns; i++) {
    int failed = pthread_mutex_init(&interns[i], NULL);
    if (failed) {
      fprintf(stderr, "Failed to initialize mutexes.\n");
      exit(1);
    }
  }

  // Initialize the barrier
  if (pthread_barrier_init(&global_barrier, NULL, num_companies + 1)) {
    fprintf(stderr, "Failed to initialize barrier.\n");
    exit(1);
  }

  pthread_t surveillance;
  if (pthread_create(&surveillance, NULL, surveillance_start, companies)) {
    fprintf(stderr, "Failed to create surveillance thread.\n");
    exit(1);
  }

  // Initializing the companies and assigning their interns
  for (int i = 0; i < num_companies; i++) {
    Company *company = companies[i];
    Company_set_company_number(company, i);
    Company_set_left_intern(company, &interns[i % num_interns]);
    Company_set_right_intern(company, &interns[(i + 1) % num_interns]);
    Company_set_job_duration(company, rand() % delta);
    Company_set_X(company,
                  40 + 20 * sin((i + 0.5) * 3.14159 * 2. / num_companies));
    Company_set_Y(company,
                  10 + 7 * cos((i + 0.5) * 3.14159 * 2. / num_companies));
    printf(ANSI_COLOR_WHITE "\033[?25l\033[%d;%dH%d joined    \n",
           (int)Company_get_Y(company), (int)Company_get_X(company),
           Company_get_company_number(company));
    pthread_t *company_thread_ptr = Company_get_thread(company);
    int failed_to_create_thread = pthread_create(
        company_thread_ptr, NULL, start_running_company, company);
    Company_set_failed_flag(company, failed_to_create_thread);
  }

  alarm(20); // Billing cycle

  if (0 != pthread_join(surveillance, NULL)) {
    fprintf(stderr, "cannot join surveillance thread.\n");
    exit(1);
  }

  // Telling the companies to finish whatever they are doing.
  for (int i = 0; i < num_companies; i++) {
    Company *company = companies[i];
    pthread_t company_thread = *(Company_get_thread(company));
    if (!Company_get_failed_flag(company) &&
        pthread_join(company_thread, NULL)) {
      // WHOOPS!
      fprintf(stderr, "error joining thread for company: %d",
              Company_get_company_number(company));
      exit(1);
    }
  }

  printf(ANSI_COLOR_RESET "\033[?25h\033[20;H");

  pthread_barrier_destroy(&global_barrier);

  printf("MAKE IT RAIN!!!\n");

  // Time to collect money from companies
  int total = 0; // Number of billable days
  for (int i = 0; i < num_companies; i++) {
    Company *company = companies[i];
    // Mo Money Mo Problems!
    printf("Company %d used our services for %d billable days.\n",
           Company_get_company_number(company),
           Company_get_billable_days(company));
    total += Company_get_billable_days(company);
  }
  printf("\nTotal Billable days : %d\n", total);
  return 0;
}
