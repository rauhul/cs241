/**
 * Lab: Utilities Unleashed
 * CS 241 - Fall 2016
 */

#include <stdio.h>
#include <stdlib.h>

void print_time_usage() {
  fprintf(stderr, "\n\t ./time <command> [args]\n\n");
  exit(2);
}

void print_env_usage() {
  fprintf(stderr, "\n\t ./env <var-list> <command-name>\n\n");
  exit(2);
}

void print_fork_failed() {
  fprintf(stderr, "Fork Failed!\n");
  exit(1);
}

void print_exec_failed() {
  fprintf(stderr, "Exec Failed!\n");
  exit(1);
}

void print_environment_change_failed() {
  fprintf(stderr, "Failed To Change Environment Variable(s)!\n");
  exit(1);
}

void display_results(char **argv, double duration) {
  char **ptr = argv + 1;
  while (*ptr) {
    printf("%s ", *ptr);
    ptr++;
  }
  printf("took %lf seconds\n", duration);
}
