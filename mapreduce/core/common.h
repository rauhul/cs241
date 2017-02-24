/**
 * MapReduce
 * CS 241 - Fall 2016
 */


#include "utils.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#define stdin_fd  0
#define stdout_fd 1

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define _LOG 0
#define LOG(format, ...) if (_LOG) { \
    fprintf(stderr, ANSI_COLOR_BLUE format ANSI_COLOR_MAGENTA "\t(%s, %s::%d)\n" ANSI_COLOR_RESET, \
    ## __VA_ARGS__, __FILE__, __func__, __LINE__); }

#pragma once

char **fifos;
int num_fifos;

int *mapper_pipes;
int *reducer_pipes;

pid_t *mapper_pids;
pid_t *reducer_pids;

void end(int exit_code);
void exec_process(char *process, int new_stdin, int new_stdout, pid_t *child_pid);
void exec_splitter(char *file, int num_piece, int piece_num, int new_stdout, pid_t *child_pid);
void exec_shuffler(int num_reducers, int new_stdin, pid_t *child_pid);
void exec_process_fifo_in(char *process, char *fifo, int new_stdout, pid_t *child_pid);
