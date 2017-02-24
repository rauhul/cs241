/**
* Lab: Utilities Unleashed
* CS 241 - Fall 2016
*/

#include <stdlib.h>
#include <stdio.h>

#include <time.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "format.h"


int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_time_usage();
        return 0;
    }

    // setup argv for the child process

    char **child_argv = malloc(argc * sizeof(char *));
    for (int idx = 0; idx < argc - 1; idx++) {
        child_argv[idx] = argv[idx + 1];
    }
    child_argv[argc - 1] = NULL;

    pid_t child = fork();

    // FORK FAILED
    if (child < 0) {
        free(child_argv);
        print_fork_failed();
    }

    // FORK SUCCEEDED

    // CHILD PROCESS
    if (child == 0) {
        int status = execvp(child_argv[0], child_argv);
        if (status) {
            free(child_argv);
            print_exec_failed();
        }
        // CHILD PROCESS DOESN'T CONTINUE PAST HERE
    }
    // PARENT PROCESS
    else {
        struct timespec start, end;
        int status;

        clock_gettime(CLOCK_MONOTONIC, &start); /* mark start time */
        waitpid(child, &status, WUNTRACED);
        clock_gettime(CLOCK_MONOTONIC, &end);   /* mark the end time */

        double duration = end.tv_sec - start.tv_sec + (end.tv_nsec - start.tv_nsec)/1000000000.0;

        if (!status)
            display_results(argv, duration);
    }

    free(child_argv);
    return 0;
}
