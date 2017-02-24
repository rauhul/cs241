/**
 * Map Reduce 0 Lab
 * CS 241 - Fall 2016
 */
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#include "utils.h"

#define LOG 0
#define _LOG 1
#define LOGL(format, ...) if (_LOG) fprintf (stderr, format, ## __VA_ARGS__)

#define stdin_fd  0
#define stdout_fd 1

void print(char *s) {
    fprintf(stderr, "%s\n", s);
}

void die() {
    if (LOG) print("> Fatal Error <");
    descriptors_closeall();
    descriptors_destroy();
    exit(0);
}

void exec_process(char *process, int new_stdin, int new_stdout, pid_t *child_pid) {
    LOGL("> exec_process %s\n", process);

    *child_pid = fork();
    if (*child_pid == -1) {
        return;
    }

    if (*child_pid == 0) {
        LOGL(">> %s: Replace stdin with %d\n", process, new_stdin);
        // close(stdin_fd);
        if (dup2(new_stdin, stdin_fd) == -1) {
            LOGL(">>> %s: Replace stdin failed", process);
            die();
        }

        LOGL(">> %s: Replace stdout with %d\n", process, new_stdout);
        // close(stdout_fd);
        if (dup2(new_stdout, stdout_fd) == -1) {
            LOGL(">>> %s: Replace stdout failed", process);
            die();
        }

        // close extra fd's
        descriptors_closeall();
        descriptors_destroy();

        LOGL(">> %s: Execute\n", process);
        execlp(process, process, NULL);
        LOGL(">> %s: Execute failed\n", process);
        die();
    }
}

int main(int argc, char **argv) {

    // Verify inputs
    if (LOG) print("> Verify inputs");
    if (argc != 5) {
        print_usage();
        return 0;
    }


    // Parse inputs
    if (LOG) print("> Parse inputs");
    char *input_file = argv[1];
    char *output_file = argv[2];
    char *mapper_executable = argv[3];
    char *reducer_executable = argv[4];
    if (LOG) fprintf(stderr, ">> %s %s %s %s\n", input_file, output_file, mapper_executable, reducer_executable);


    // Open the input file.
    if (LOG) print("> Open the input file");
    int input_fd = open(input_file, O_RDONLY);
    if (input_fd == -1) {
        if (LOG) print(">> Open input file failed");
        die();
    }
    descriptors_add(input_fd);


    // Create a pipe to connect the mapper to the reducer.
    if (LOG) print("> Create a pipe to connect the mapper to the reducer.");
    int pipe_fd[2];
    if (pipe2(pipe_fd, O_CLOEXEC) == -1) {
        if (LOG) print(">> Create pipe failed");
        die();
    }
    descriptors_add(pipe_fd[0]);
    descriptors_add(pipe_fd[1]);


    // Open the output file.
    if (LOG) print("> Open the output file.");
    int output_fd = open(output_file, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (output_fd == -1) {
        if (LOG) print(">> Open output file failed");
        die();
    }
    descriptors_add(output_fd);


    // Start the mapper.
    if (LOG) print("> Start the mapper.");

    pid_t mapper_pid;
    exec_process(mapper_executable, input_fd, pipe_fd[1], &mapper_pid);
    if (mapper_pid == -1) {
        if (LOG) print(">> Fork failed");
        die();
    }

    // Start the reducer.
    if (LOG) print("> Start the reducer.");

    pid_t reducer_pid;
    exec_process(reducer_executable, pipe_fd[0], output_fd, &reducer_pid);
    if (reducer_pid == -1) {
        if (LOG) print(">> Fork failed");
        die();
    }

    // Clean up.
    descriptors_closeall();
    descriptors_destroy();

    // Wait for the reducer to finish.
    if (LOG) print("> Wait for the reducer to finish.");

    int mapper_status;
    if (waitpid(mapper_pid, &mapper_status, 0) == -1 ) {
        if (LOG) print(">> Waitpid map failed");
        die();
    }

    int reducer_status;
    if (waitpid(reducer_pid, &reducer_status, 0) == -1 ) {
        if (LOG) print(">> Waitpid reduce failed");
        die();
    }


    // Print nonzero subprocess exit codes.
    if (LOG) print("> Print nonzero subprocess exit codes.");

    if (WIFEXITED(mapper_status)) {
        int exit_code = WEXITSTATUS(mapper_status);
        fprintf(stdout, "my_mapper exited with status %d\n", exit_code);
    }

    if (WIFEXITED(reducer_status)) {
        int exit_code = WEXITSTATUS(reducer_status);
        fprintf(stdout, "my_reducer exited with status %d\n", exit_code);
    }


    // Count the number of lines in the output file.
    if (LOG) print("> Count the number of lines in the output file.");
    print_num_lines(output_file);

    return 0;
}
