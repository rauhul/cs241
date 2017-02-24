/**
 * MapReduce
 * CS 241 - Fall 2016
 */

#include "common.h"

int main(int argc, char **argv) {
    num_fifos = 0;
    // Verify inputs
    LOG("> Verify inputs")
    if (argc != 6) {
        print_mr1_usage();
        end(0);
    }

    // Parse inputs
    LOG("> Parse inputs")
    char *input_file            = argv[1];
    char *output_file           = argv[2];
    char *mapper_executable     = argv[3];
    char *reducer_executable    = argv[4];
    int num_mappers             = atoi(argv[5]);
    LOG(">> %s %s %s %s %d", input_file, output_file, mapper_executable, reducer_executable, num_mappers)

    // Open the output file.
    LOG("> Open the output file.")
    int output_fd = open(output_file, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (output_fd == -1) {
        LOG(">> Open output file failed")
        end(1);
    }
    descriptors_add(output_fd);

    // Create an input pipe for each mapper.
    LOG("> Create an input pipe for each mapper.")
    mapper_pipes = malloc(2 * num_mappers * sizeof(int));
    for (int i = 0; i < num_mappers; i++) {
        int index = i * 2;
        if (pipe2(mapper_pipes + index, O_CLOEXEC) == -1) {
            LOG(">> Create pipe failed")
            end(1);
        }
        descriptors_add(mapper_pipes[index]);
        descriptors_add(mapper_pipes[index + 1]);
    }

    // Create one input pipe for the reducer.
    LOG("> Create an input pipe for the reducer.")
    int reducer_pipe[2];
    if (pipe2(reducer_pipe, O_CLOEXEC) == -1) {
        LOG(">> Create pipe failed")
        end(1);
    }
    descriptors_add(reducer_pipe[0]);
    descriptors_add(reducer_pipe[1]);

    // Start a splitter process for each mapper.
    for (int i = 0; i < num_mappers; i++) {
        pid_t splitter_pid;
        exec_splitter(input_file, num_mappers, i, mapper_pipes[(i * 2) + 1], &splitter_pid);
    }

    // Start all the mapper processes.
    mapper_pids = malloc(num_mappers * sizeof(pid_t));
    for (int i = 0; i < num_mappers; i++) {
        exec_process(mapper_executable, mapper_pipes[i * 2], reducer_pipe[1], mapper_pids + i);
    }

    // Start the reducer process.
    pid_t reducer_pid;
    exec_process(reducer_executable, reducer_pipe[0], output_fd, &reducer_pid);

    descriptors_closeall();
    descriptors_destroy();

    // Wait for the reducer to finish.
    LOG("> Wait for the reducer to finish.")
    int reducer_status;
    if (waitpid(reducer_pid, &reducer_status, 0) == -1) {
        LOG(">> Waitpid reduce failed");
        end(3);
    }

    // Print nonzero subprocess exit codes.
    LOG("> Print nonzero subprocess exit codes.")
    for (int i = 0; i < num_mappers; i++) {
        int mapper_status;
        if (waitpid(mapper_pids[i], &mapper_status, 0) == -1) {
            LOG(">> Waitpid map failed")
            end(3);
        }
        if (WIFEXITED(mapper_status)) {
            int exit_code = WEXITSTATUS(mapper_status);
            if (exit_code != 0) {
                fprintf(stdout, "%s exited with status %d\n", mapper_executable, exit_code);
            }
        }
    }

    if (WIFEXITED(reducer_status)) {
        int exit_code = WEXITSTATUS(reducer_status);
        if (exit_code != 0) {
            fprintf(stdout, "%s exited with status %d\n", reducer_executable, exit_code);
        }
    }

    // Count the number of lines in the output file.
    LOG("> Count the number of lines in the output file.")
    print_num_lines(output_file);

    // Clean up
    LOG("> Clean up")
    if (mapper_pipes)
        free(mapper_pipes);
    if (mapper_pids)
        free(mapper_pids);

    return 0;
}
