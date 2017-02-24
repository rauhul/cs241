/**
 * MapReduce
 * CS 241 - Fall 2016
 */

#include "common.h"

void end(int exit_code) {
    LOG("> Fatal Error <");
    descriptors_closeall();
    descriptors_destroy();

    // system("rm ./fifo_*");

    if (fifos) {
        for (int i = 0; i < num_fifos; i++) {
            free(fifos[i]);
        }
        free(fifos);
    }

    if (mapper_pipes)
        free(mapper_pipes);

    if (mapper_pids)
        free(mapper_pids);

    if (reducer_pipes)
        free(reducer_pipes);

    if (reducer_pids)
        free(reducer_pids);

    exit(exit_code);
}

void exec_process(char *process, int new_stdin, int new_stdout, pid_t *child_pid) {
    LOG("> exec_process %s", process)

    *child_pid = fork();
    if (*child_pid == -1) {
        LOG(">> Fork failed")
        end(2);
    }

    if (*child_pid == 0) {
        LOG(">> %s: Replace stdin with %d", process, new_stdin)
        if (dup2(new_stdin, stdin_fd) == -1) {
            LOG(">>> %s: Replace stdin failed", process)
            end(2);
        }

        LOG(">> %s: Replace stdout with %d", process, new_stdout)
        if (dup2(new_stdout, stdout_fd) == -1) {
            LOG(">>> %s: Replace stdout failed", process)
            end(2);
        }

        // close extra fd's
        descriptors_closeall();
        descriptors_destroy();

        LOG(">> %s: Execute", process)
        execlp(process, process, NULL);
        LOG(">>> %s: Execute failed", process)
        end(1);
    }
}

void exec_process_fifo_in(char *process, char *fifo, int new_stdout, pid_t *child_pid) {
    LOG("> exec_process %s", process)

    *child_pid = fork();
    if (*child_pid == -1) {
        LOG(">> Fork failed")
        end(2);
    }

    if (*child_pid == 0) {

        LOG(">> %s: Replace stdin with fifo: %s", process, fifo)
        int new_stdin = open(fifo, O_RDONLY);
        if (new_stdin == -1) {
            LOG(">>> open failed")
            end(200);
        }

        if (dup2(new_stdin, stdin_fd) == -1) {
            LOG(">>> %s: Replace stdin failed", process)
            end(2);
        }

        LOG(">> %s: Replace stdout with %d", process, new_stdout)
        if (dup2(new_stdout, stdout_fd) == -1) {
            LOG(">>> %s: Replace stdout failed", process)
            end(2);
        }

        // close extra fd's
        descriptors_closeall();
        descriptors_destroy();

        LOG(">> %s: Execute", process)
        execlp(process, process, NULL);
        LOG(">>> %s: Execute failed", process)
        end(1);
    }
}


void exec_splitter(char *file, int num_piece, int piece_num, int new_stdout, pid_t *child_pid) {
    char *process = "./splitter";
    LOG("> exec_process %s", process)

    *child_pid = fork();
    if (*child_pid == -1) {
        LOG(">> Fork failed")
        end(2);
    }

    if (*child_pid == 0) {
        LOG(">> %s: Replace stdout with %d", process, new_stdout)
        if (dup2(new_stdout, stdout_fd) == -1) {
            LOG(">>> %s: Replace stdout failed", process)
            end(2);
        }

        // close extra fd's
        descriptors_closeall();
        descriptors_destroy();

        LOG(">> %s: Execute", process)
        char arg1[10];
        snprintf(arg1, 10, "%d", num_piece);
        char arg2[10];
        snprintf(arg2, 10, "%d", piece_num);

        execlp("./splitter", "./splitter", file, arg1, arg2, NULL);
        LOG(">>> %s: Execute failed", process)
        end(1);
    }
}

void exec_shuffler(int num_reducers, int new_stdin, pid_t *child_pid) {
    char *process = "./shuffler";
    LOG("> exec_process %s", process)

    *child_pid = fork();
    if (*child_pid == -1) {
        LOG(">> Fork failed")
        end(2);
    }

    if (*child_pid == 0) {
        LOG(">> %s: Replace stdin with %d", process, new_stdin)
        if (dup2(new_stdin, stdin_fd) == -1) {
            LOG(">>> %s: Replace stdin failed", process)
            end(2);
        }

        // close extra fd's
        descriptors_closeall();
        descriptors_destroy();

        char **shuffler_argv = malloc(sizeof(char *) * (num_reducers + 2));
        shuffler_argv[0] = strdup(process);
        shuffler_argv[num_reducers + 1] = NULL;

        for (int i = 0; i < num_reducers; i++) {
            int idx = i + 1;
            shuffler_argv[idx] = malloc(sizeof(char) * 10);
            snprintf(shuffler_argv[idx], 10, "fifo_%d", i);
        }

        LOG(">> %s: Execute", process)
        execvp("./shuffler", shuffler_argv);

        for (int i = 0; i < num_reducers + 2; i++) {
            LOG("DEBUG::shuffler_argv[%d]=%s", i, shuffler_argv[i])
        }
        for (int i = 0; i < num_reducers + 1; i++) {
            free(shuffler_argv[i]);
        }
        free(shuffler_argv);

        LOG(">>> %s: Execute failed", process)
        end(1);
    }
}