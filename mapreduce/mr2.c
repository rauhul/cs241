/**
 * MapReduce
 * CS 241 - Fall 2016
 */

#include "common.h"

int main(int argc, char **argv) {
    num_fifos = 0;
    // Verify inputs
    LOG("> Verify inputs")
    if (argc != 7) {
        print_mr2_usage();
        end(0);
    }

    // Parse inputs
    LOG("> Parse inputs")
    char *input_file            = argv[1];
    char *output_file           = argv[2];
    char *mapper_executable     = argv[3];
    char *reducer_executable    = argv[4];
    int num_mappers             = atoi(argv[5]);
    int num_reducers            = atoi(argv[6]);
    LOG(">> %s %s %s %s %d %d", input_file, output_file, mapper_executable, reducer_executable, num_mappers, num_reducers)

    // Open the output file.
    LOG("> Open the output file.")
    int output_fd = open(output_file, O_APPEND | O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
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

    // Create one input pipe for the shuffler.
    LOG("> Create an input pipe for the shuffler.")
    int shuffler_pipe[2];
    if (pipe2(shuffler_pipe, O_CLOEXEC) == -1) {
        LOG(">> Create pipe failed")
        end(1);
    }
    descriptors_add(shuffler_pipe[0]);
    descriptors_add(shuffler_pipe[1]);

    // Make named pipes to use with the shuffler and reducers
    LOG("> Make named pipes to use with the shuffler and reducers.")
    fifos = malloc(num_reducers * sizeof(char *));
    num_fifos = num_reducers;
    for (int i = 0; i < num_reducers; i++) {
        char fifo[10];
        snprintf(fifo, 10, "fifo_%d", i);
        fifos[i] = strdup(fifo);

        LOG(">> unlink %s", fifo)
        unlink(fifo);

        LOG(">> mkfifo %s", fifo)
        if (mkfifo(fifo, S_IRUSR | S_IWUSR) == -1) {
            LOG(">> mkfifo failed")
            end(2);
        }
    }

    // Start a splitter process for each mapper.
    for (int i = 0; i < num_mappers; i++) {
        pid_t splitter_pid;
        exec_splitter(input_file, num_mappers, i, mapper_pipes[(i * 2) + 1], &splitter_pid);
    }

    // Start all the mapper processes.
    mapper_pids = malloc(num_mappers * sizeof(pid_t));
    for (int i = 0; i < num_mappers; i++) {
        exec_process(mapper_executable, mapper_pipes[i * 2], shuffler_pipe[1], mapper_pids + i);
    }

    // Start the shuffler process.
    pid_t shuffler_pid;
    exec_shuffler(num_reducers, shuffler_pipe[0], &shuffler_pid);

    // Start all the reducer processes.
    reducer_pids = malloc(num_reducers * sizeof(pid_t));
    for (int i = 0; i < num_reducers; i++) {
        exec_process_fifo_in(reducer_executable, fifos[i], output_fd, reducer_pids + i);
    }

    // close extra fd's
    descriptors_closeall();
    descriptors_destroy();

    // Wait for the everything to finish.
    LOG("> Wait for the everything to finish.")
    LOG("> Print nonzero subprocess exit codes.")
    int reducer_status;
    for (int i = 0; i < num_reducers; i++) {
        if (waitpid(reducer_pids[i], &reducer_status, 0) == -1) {
            LOG(">> Waitpid reduce failed");
            end(3);
        }
        if (WIFEXITED(reducer_status)) {
            int exit_code = WEXITSTATUS(reducer_status);
            if (exit_code != 0) {
                fprintf(stdout, "%s exited with status %d\n", reducer_executable, exit_code);
            }
        }
    }

    int mapper_status;
    for (int i = 0; i < num_mappers; i++) {
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

    int shuffler_status;
    if (waitpid(shuffler_pid, &shuffler_status, 0) == -1) {
        LOG(">> Waitpid shuffle failed")
        end(3);
    }

    if (_LOG && WIFEXITED(shuffler_status)) {
        int exit_code = WEXITSTATUS(shuffler_status);
        LOG("shuffler exited with status %d\n", exit_code);
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
    if (reducer_pipes)
        free(reducer_pipes);
    if (reducer_pids)
        free(reducer_pids);

    system("rm ./fifo_*");

    if (fifos) {
        for (int i = 0; i < num_fifos; i++) {
            free(fifos[i]);
        }
        free(fifos);
    }

    return 0;
}
