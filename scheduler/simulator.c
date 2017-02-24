/**
 * Scheduler Lab
 * CS 241 - Fall 2016
 */
/*
 * CS 241
 * The University of Illinois
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "libscheduler.h"

typedef struct _simulator_job_list_t {
  int job_id, arrival_time, run_time, priority;
  int core_id, arrived;
} simulator_job_list_t;

void print_usage(char *program_name) {
  fprintf(stderr, "Usage: %s -c <cores> -s <scheme> <input file>\n",
          program_name);
  fprintf(stderr, "       %s -c 2 -s fcfs examples/proc1.csv\n", program_name);
  fprintf(stderr, "\n");
  fprintf(stderr, "Acceptable schemes are: fcfs, sjf, psjf, pri, ppri, rr#\n");
}

int set_active_job(int job_id, int core_id, simulator_job_list_t *jobs,
                   int active_jobs) {
  int i;
  for (i = 0; i < active_jobs; i++) {
    if (jobs[i].job_id == job_id && jobs[i].arrived) {
      jobs[i].core_id = core_id;
      return 1;
    }
  }

  return 0;
}

void print_available_jobs(simulator_job_list_t *jobs, int active_jobs) {
  printf("Active jobs are: ");

  int i, first = 1;
  for (i = 0; i < active_jobs; i++) {
    if (jobs[i].arrived) {
      if (first) {
        printf("%d", jobs[i].job_id);
        first = 0;
      } else
        printf(", %d", jobs[i].job_id);
    }
  }

  if (!first)
    printf("\n");
}

void print_available_cores(int cores) {
  printf("Active cores are: ");

  int i;
  for (i = 0; i < cores; i++) {
    if (i == cores - 1)
      printf("%d\n", i);
    else
      printf("%d, ", i);
  }
}

int main(int argc, char **argv) {
  int c;
  int cores = 0, scheme = -1, quantum = 0;
  char *file_name;

  /*
   * Parse command line options.
   */
  while ((c = getopt(argc, argv, "c:s:")) != -1) {
    switch (c) {
    case 'c':
      cores = atoi(optarg);

      if (cores <= 0) {
        fprintf(stderr, "Option -c <cores> require a positive number.\n");
        print_usage(argv[0]);
        return 1;
      }
      break;

    case 's':
      if (strcasecmp(optarg, "FCFS") == 0) {
        scheme = FCFS;
      } else if (strcasecmp(optarg, "PPRI") == 0) {
        scheme = PPRI;
      } else if (strcasecmp(optarg, "PRI") == 0) {
        scheme = PRI;
      } else if (strcasecmp(optarg, "PSJF") == 0) {
        scheme = PSJF;
      } else if (strncasecmp(optarg, "RR", 2) == 0) {
        scheme = RR;
        quantum = atoi(optarg + 2);

        if (quantum <= 0) {
          fprintf(stderr, "Option -s <scheme> requires a positive number for "
                          "the quantum of RR. (Eg: -s RR2)\n");
          print_usage(argv[0]);
          return 1;
        }
      } else if (strcasecmp(optarg, "SJF") == 0) {
        scheme = SJF;
      }
      break;

    case '?':
      print_usage(argv[0]);
      return 1;

    default:
      printf("...\n");
      break;
    }
  }

  if (cores == 0) {
    fprintf(stderr, "Required option -c <cores> is not present.\n");
    print_usage(argv[0]);
    return 1;
  }

  if (scheme == -1) {
    fprintf(stderr, "Required option -s <scheme> is not present.\n");
    print_usage(argv[0]);
    return 1;
  }

  if (optind == argc - 1)
    file_name = argv[optind];
  else {
    fprintf(stderr, "A single input file is required.\n");
    print_usage(argv[0]);
    return 1;
  }

  /*
   * Open the file, read the file, and populate the jobs data structure.
   */
  FILE *file = fopen(file_name, "r");
  if (file == NULL) {
    fprintf(stderr, "Unable to open file \"%s\".\n", file_name);
    return 2;
  }

  int job_id = 0;
  int jobs_ct = 10;
  simulator_job_list_t *jobs = malloc(jobs_ct * sizeof(simulator_job_list_t));

  char line[1024 + 1];
  fgets(line, 1024, file); // Ignore the first (header) line
  while (fgets(line, 1024, file) != NULL) {
    char *arrival_time = strtok(line, ",");
    char *run_time = strtok(NULL, ",");
    char *priority = strtok(NULL, ",");

    if (arrival_time != NULL && run_time != NULL && priority != NULL) {
      if (job_id == jobs_ct) {
        jobs_ct *= 2;
        jobs = realloc(jobs, jobs_ct * sizeof(simulator_job_list_t));

        if (!jobs) {
          fprintf(stderr, "Out of memory.\n");
          return 2;
        }
      }

      jobs[job_id].job_id = job_id;
      jobs[job_id].arrival_time = atoi(arrival_time);
      jobs[job_id].run_time = atoi(run_time);
      jobs[job_id].priority = atoi(priority);
      jobs[job_id].core_id = -1;
      jobs[job_id].arrived = 0;

      job_id++;
    } else {
      fprintf(stderr, "Illegal file format.\n");
      return 2;
    }
  }

  fclose(file);

  /*
   * Run the simulation.
   */

  printf("Loaded %d core(s) and %d job(s) using ", cores, job_id);
  if (scheme == FCFS) {
    printf("First Come First Served (FCFS)");
  } else if (scheme == PSJF) {
    printf("Preemptive Shortest Job First (PSJF)");
  } else if (scheme == PRI) {
    printf("Non-preemptive Priority (PRI)");
  } else if (scheme == RR) {
    printf("Round Robin (RR) with a quantum of %d", quantum);
  }
  printf(" scheduling...\n\n");

  scheduler_start_up(cores, scheme);

  int time = 0, i, j;
  int active_jobs = job_id, jobs_alive = 0;

  int *quantum_clock = malloc(cores * sizeof(int));
  char **core_timing_diagram = malloc(cores * sizeof(char *));
  int core_timing_diagram_size = 1024;

  for (i = 0; i < cores; i++) {
    quantum_clock[i] = -1;
    core_timing_diagram[i] = malloc(core_timing_diagram_size + 1);
    core_timing_diagram[i][0] = '\0';
  }

  while (active_jobs > 0) {
    printf("=== [TIME %d] ===\n", time);

    /*
     * 1. Check if any jobs finished in the last time unit.
     */
    for (i = 0; i < active_jobs; i++) {
      // .run_time is >0 if job hasn't been completed yet
      // ==0 if it JUST finished
      // ==-1 if it is already done
      if (jobs[i].run_time == 0) {
        // Notify the scheduler has finished
        int job_id = jobs[i].job_id;
        int core_id = jobs[i].core_id;
        int new_job_id =
            scheduler_job_finished(jobs[i].core_id, jobs[i].job_id, time);

        if (scheme == RR)
          quantum_clock[jobs[i].core_id] = quantum;

        // Delete the finished jobs, decrease the number of active jobs
        if (i != active_jobs - 1)
          memcpy(&jobs[i], &jobs[active_jobs - 1],
                 sizeof(simulator_job_list_t));
        active_jobs--;
        jobs_alive--;
        i--;

        // Set the new job
        if (new_job_id != -1 &&
            !set_active_job(new_job_id, core_id, jobs, active_jobs)) {
          printf("The scheduler_job_finished() selected an invalid job (job_id "
                 "== %d).\n",
                 new_job_id);
          print_available_jobs(jobs, active_jobs);
          return 3;
        } else {
          printf("Job %d, running on core %d, finished. Core %d is now running "
                 "job %d.\n",
                 job_id, core_id, core_id, new_job_id);
          printf("  Queue: ");
          scheduler_show_queue();
          printf("\n\n");
        }
      }
    }

    /*
     * Check to see if we finished our last job.  (If we don't check here, we
     * would run an extra time unit that will be totally idle.)
     */
    if (active_jobs == 0)
      break;

    /*
     * 2. Check of any quantums expired in the last time unit.
     */
    if (scheme == RR) {
      for (i = 0; i < cores; i++) {
        if (quantum_clock[i] == 0) {
          for (j = 0; j < active_jobs; j++) {
            if (jobs[j].core_id == i) {
              // Notify the scheduler the quantum has expired
              int core_id = jobs[j].core_id;
              int old_job_id = jobs[j].job_id;
              int new_job_id = scheduler_quantum_expired(jobs[j].core_id, time);

              jobs[j].core_id = -1;

              quantum_clock[core_id] = quantum;

              // Set the new job
              if (new_job_id != -1 &&
                  !set_active_job(new_job_id, core_id, jobs, active_jobs)) {
                printf("The scheduler_quantum_expired() selected an invalid "
                       "job (job_id == %d).\n",
                       new_job_id);
                print_available_jobs(jobs, active_jobs);
                return 3;
              } else {
                printf("Job %d, running on core %d, had its quantum expire. "
                       "Core %d is now running job %d.\n",
                       old_job_id, core_id, core_id, new_job_id);
                printf("  Queue: ");
                scheduler_show_queue();
                printf("\n\n");
              }

              break;
            }
          }
        }
      }
    }

    /*
     * 3. Check for any new jobs that arrive in this time unit
     */
    for (i = 0; i < active_jobs; i++) {
      if (jobs[i].arrival_time == time) {
        int new_job_core_id = scheduler_new_job(
            jobs[i].job_id, time, jobs[i].run_time, jobs[i].priority);
        jobs[i].arrived = 1;
        jobs_alive++;

        if (new_job_core_id >= 0 && new_job_core_id < cores) {
          printf("A new job, job %d (running time=%d, priority=%d), arrived. "
                 "Job %d is now running on core %d.\n",
                 jobs[i].job_id, jobs[i].run_time, jobs[i].priority,
                 jobs[i].job_id, new_job_core_id);
          printf("  Queue: ");
          scheduler_show_queue();
          printf("\n\n");

          // Find if anyone is currently using the core.
          for (j = 0; j < active_jobs; j++)
            if (jobs[j].core_id == new_job_core_id)
              jobs[j].core_id = -1;

          // Assign the core to the new job
          jobs[i].core_id = new_job_core_id;

          if (scheme == RR)
            quantum_clock[new_job_core_id] = quantum;
        } else if (new_job_core_id == -1) {
          printf("A new job, job %d (running time=%d, priority=%d), arrived. "
                 "Job %d is set to idle (-1).\n",
                 jobs[i].job_id, jobs[i].run_time, jobs[i].priority,
                 jobs[i].job_id);
          printf("  Queue: ");
          scheduler_show_queue();
          printf("\n\n");
        } else {
          printf("The scheduler_new_job() selected an invalid core (core_id == "
                 "%d).\n",
                 new_job_core_id);
          print_available_cores(cores);
          return 3;
        }
      }
    }

    /*
     * 4. Run the time unit.
     */
    char time_string[cores][11];
    int cores_working = 0;

    for (i = 0; i < cores; i++)
      time_string[i][0] = '\0';

    for (i = 0; i < active_jobs; i++) {
      if (jobs[i].core_id != -1) {
        cores_working++;
        jobs[i].run_time--;
        quantum_clock[jobs[i].core_id]--;

        assert(time_string[jobs[i].core_id][0] == '\0');

        if (jobs[i].job_id < 10)
          sprintf(time_string[jobs[i].core_id], "%d", jobs[i].job_id);
        else if (jobs[i].job_id < 10 + 26)
          sprintf(time_string[jobs[i].core_id], "%c",
                  jobs[i].job_id - 10 + 'a');
        else if (jobs[i].job_id < 10 + 26 + 26)
          sprintf(time_string[jobs[i].core_id], "%c",
                  jobs[i].job_id - 10 - 26 + 'A');
        else
          snprintf(time_string[jobs[i].core_id], 10, "(%d)", jobs[i].job_id);
      }
    }

    for (i = 0; i < cores; i++) {
      // If the core is idle, print a '-'
      if (time_string[i][0] == '\0')
        strcpy(time_string[i], "-");

      // Ensure we have enough memory
      while (strlen(core_timing_diagram[i]) + strlen(time_string[i]) >=
             (unsigned int)core_timing_diagram_size) {
        core_timing_diagram_size *= 2;

        for (j = 0; j < cores; j++) {
          core_timing_diagram[j] =
              realloc(core_timing_diagram[j], core_timing_diagram_size + 1);

          if (core_timing_diagram[j] == NULL) {
            fprintf(stderr, "Out of memory.\n");
            return 3;
          }
        }
      }

      strcat(core_timing_diagram[i], time_string[i]);
    }

    /*
     * 5. Print data!
     */
    printf("At the end of time unit %d...\n", time);

    for (i = 0; i < cores; i++)
      printf("  Core %2d: %s\n", i, core_timing_diagram[i]);

    printf("\n");

    printf("  Queue: ");
    scheduler_show_queue();
    printf("\n");
    printf("\n");

    /*
     * 6. Sanity Checking
     *
     * - If there's a job alive (needing to be ran) and all CPUs are idle, the
     * scheduler failed to schedule properly.
     */
    if (jobs_alive > 0 && cores_working == 0) {
      printf("All cores are idle and at least one job remains unscheduled.\n");
      print_available_jobs(jobs, active_jobs);
      return 3;
    }

    /*
     * 7. Increase time
     */
    time++;
  }

  printf("FINAL TIMING DIAGRAM:\n");
  for (i = 0; i < cores; i++)
    printf("  Core %2d: %s\n", i, core_timing_diagram[i]);

  printf("\n");
  printf("Average Waiting Time: %.2f\n", scheduler_average_waiting_time());
  printf("Average Turnaround Time: %.2f\n",
         scheduler_average_turnaround_time());
  printf("Average Response Time: %.2f\n", scheduler_average_response_time());

  scheduler_clean_up();

  free(quantum_clock);
  for (i = 0; i < cores; i++)
    free(core_timing_diagram[i]);
  free(core_timing_diagram);
  free(jobs);

  return 0;
}
