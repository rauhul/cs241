/**
 * Machine Problem: Malloc
 * CS 241 - Fall 2016
 */

#include "contest.h"

#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int child_still_running = 1;

void *timeout_timer(void *ptr) {
  int child_pid = *((int *)ptr);

  sleep(TIMER_TIMEOUT);
  if (child_still_running == 1) {
    printf("Sending a SIGTERM to kill the child process (pid=%d) for running "
           "for over %dsec.\n",
           child_pid, TIMER_TIMEOUT);
    if (kill(child_pid, SIGTERM) != 0)
      if (errno != ESRCH)
        perror("kill()");
  }

  return NULL;
}

int main(int argc, char **argv) {
  /*
   * Check to ensure that the program is launched with at least one command
   * line option.  Display helpful text if no options are present.
   */
  if (argc == 1) {
    printf("You must supply a program to be invoked to use your replacement "
           "malloc() script.\n");
    printf("...you may use any program, even system programs, such as `ls`.\n");
    printf("\n");
    printf("Example: %s /bin/ls\n", argv[0]);
    return 1;
  }

  /*
   * Set up a shared memory file for later use by mmap().
   */
  char file_name[] = "/tmp/cs241-XXXXXX";
  int fd = mkstemp(file_name);

  char *buffer = calloc(1, sizeof(alloc_stats_t));
  write(fd, buffer, sizeof(alloc_stats_t));
  close(fd);
  free(buffer);

  /*
   * Set up the environment to pre-load our 'malloc.so' shared library, which
   * will replace the malloc(), calloc(), realloc(), and free() that is defined
   * by standard libc.
   */
  char **env = malloc(3 * sizeof(char *));
  env[0] = malloc(1024 * sizeof(char));
  sprintf(env[0], "LD_PRELOAD=./contest-alloc.so");

  env[1] = malloc(1024 * sizeof(char));
  sprintf(env[1], "ALLOC_CONTEST_MMAP=%s", file_name);

  env[2] = NULL;

  /*
   * Replace the current running process with the process specified by the
   * command
   * line options.  If exec() fails, we won't even try and recover as there's
   * likely
   * nothing we could really do; however, we do our best to provide useful
   * output
   * with a call to perror().
   */
  int forkid = fork();
  if (forkid == 0) /* child */
  {
    execve(argv[1], argv + 1,
           env); /* Note that exec() will not return on success. */
    perror("exec() failed");
    return 3;
  }
  free(env[1]);
  free(env[0]);
  free(env);

  pthread_t tid;
  pthread_create(&tid, NULL, timeout_timer, &forkid);
  child_still_running = 1;

  int result;
  struct rusage resources_used;
  if (wait4(forkid, &result, 0, &resources_used) == -1) {
    perror("wait4()");
    return 4;
  }
  child_still_running = 0;
  pthread_detach(tid);

  FILE *file = fopen(file_name, "r");
  alloc_stats_t *stats =
      mmap(NULL, sizeof(alloc_stats_t), PROT_READ, MAP_SHARED, fileno(file), 0);
  if (!stats) {
    perror("mmap");
    return 5;
  }

  fclose(file);

  int total_sec =
      resources_used.ru_utime.tv_sec + resources_used.ru_stime.tv_sec;
  int total_usec =
      resources_used.ru_utime.tv_usec + resources_used.ru_stime.tv_usec;
  if (total_usec >= 1000 * 1000) {
    total_usec -= 1000 * 1000;
    total_sec++;
  }
  double total_time = total_sec + ((double)total_usec / ((double)1000 * 1000));

  if (result == 0)
    printf("[mcontest]: STATUS: OK\n");
  else
    printf("[mcontest]: STATUS: FAILED=(%d)\n", result);

  printf("[mcontest]: MAX: %llu\n", stats->max_heap_used);

  if (stats->memory_uses == 0)
    printf("[mcontest]: AVG: %f\n", 0.0f);
  else
    printf("[mcontest]: AVG: %f\n",
           (stats->memory_heap_sum / (double)stats->memory_uses));

  printf("[mcontest]: TIME: %f\n", total_time);

  munmap(stats, sizeof(alloc_stats_t));
  unlink(file_name);
  return 0;
}
