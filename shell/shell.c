/**
 * Machine Problem: Shell
 * CS 241 - Fall 2016
 */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#include "log.h"
#include "shell.h"
#include "format.h"

void parse_command(const char *command);

// RUN ulimit -u 10 before anything

// ALLOCATED ELEMENTS
Log  *history           = NULL;
Log  *commands          = NULL;
char *command_file_name = NULL;
char *history_file_name = NULL;
char *history_file_path = NULL;

// MEMORY CLEANING
#define end free_shell(); return 0;
#define end_hard free_shell(); exit(EXIT_FAILURE);

void free_file_names() {
    if (command_file_name)
        free(command_file_name);
    if (history_file_name)
        free(history_file_name);
}

void free_history() {
    if (history) {
        if (history_file_path) {
            Log_save(history, history_file_path);
            free(history_file_path);
        }
        Log_destroy(history);
    }
}

void free_commands() {
    if (commands) {
        Log_destroy(commands);
    }
}

void free_shell() {
    free_file_names();
    free_history();
    free_commands();
}

// RUNNING COMMANDS

//! prefix
void exec_history_command() {
    size_t history_len = Log_size(history);

    const char *command;
    for (size_t iter = 0; iter < history_len; iter++) {
        command = Log_get_command(history, iter);
        print_history_line(iter, command);
    }
}

void exec_search_command(const char *prefix) {
    size_t history_len = Log_size(history);

    prefix++;
    size_t prefix_len = strlen(prefix);
    size_t iter;
    const char *command;
    for (size_t i = 1; i <= history_len; i++) {
        iter = history_len - i;
        command = Log_get_command(history, iter);
        if (!strncmp(prefix, command, prefix_len)) {
            print_command(command);
            parse_command(command);
            return;
        }
    }

    print_no_history_match();
}

void parse_exclamation_command(const char *command) {
    if (!strcmp(command, "!history"))
        exec_history_command();
    else
        exec_search_command(command);
}

//# prefix
void exec_pound_command(const char *number) {
    number++;
    size_t number_len = strlen(number);

    if (!number_len) {
        print_invalid_index();
        return;
    }

    for (size_t iter = 0; iter < number_len; iter++) {
        if (!isdigit(number[iter])) {
            print_invalid_index();
            return;
        }
    }

    size_t line_num = atoi(number);
    size_t history_len = Log_size(history);
    if (line_num < history_len) {
        const char *command = Log_get_command(history, line_num);
        print_command(command);
        parse_command(command);
        return;
    }

    print_invalid_index();
}

//cd prefix
void exec_cd_command(const char *command) {
    size_t command_len = strlen(command);
    if (command_len < 3) {
        print_no_directory("");
        return;
    }
    command += 3;
 
    if (chdir(command)) {
        Log_add_command(history, command);
        print_no_directory(command);
    }
}

//external commands
void exec_external_command(const char *command) {
    size_t command_len    = strlen(command);
    Log_add_command(history, command);

    int async = 0;

    if (command[command_len-1] == '&')
        async = 1;

    pid_t child_pid = fork();

    if (child_pid < 0) { // failure
        print_fork_failed();
        return;
    }

    if (child_pid == 0) { // child
        char *mutable_command = malloc( (command_len + 1)*sizeof(char) );
        strcpy(mutable_command, command);

        if (mutable_command[command_len-1] == '&') {
            mutable_command[command_len-1] = '\0';
            if (command_len > 1 && mutable_command[command_len-2] == ' ')
                mutable_command[command_len-1] = '\0';
        }

        size_t numtokens;
        char **argv = strsplit(mutable_command, " ", &numtokens);
        free(mutable_command);

        if (numtokens == 0 || !argv) {
            print_exec_failed(command);
            if (argv)
                free_args(argv);
            end_hard
        }
        print_command_executed(getpid());

        execvp(argv[0], argv);

        print_exec_failed(command);
        free_args(argv);
        end_hard
    }

    if (async) {
        return;
    }

    int status;
    if (waitpid(child_pid, &status, 0) < 0) {
        print_wait_failed();
    }
}

// 
void parse_command(const char *command) {
    size_t command_len = strlen(command);

    if (command_len == 0)
        return;

    if (command[0] == '!') {
        parse_exclamation_command(command);
        return;
    }

    if (command[0] == '#') {
        exec_pound_command(command);
        return;
    }

    if (command_len > 1) {
        if (command[0] == 'c' && command[1] == 'd') {
            exec_cd_command(command);
            return;
        }
    }

    exec_external_command(command);
}

// INGORE SIGINT
void sig_handler(int sig) { 
    switch (sig) {
    case SIGINT:
        break;
    case SIGCHLD:
        while (waitpid((pid_t)(-1), 0, WNOHANG) > 0);
        break;
    default:
        break;
    }
}

// MAIN SHELL
int shell(int argc, char *argv[]) {

    signal(SIGINT,  sig_handler);
    signal(SIGCHLD, sig_handler);

    // VERIFY ARGC LENGTH
    if (!(argc == 1 || argc == 3 || argc == 5)) {
        print_usage();
        end
    }

    // GET INPUT ARGS
    opterr = 0;
    int c;

    while ((c = getopt(argc, argv, "h:f:")) != -1) {
        switch (c) {
        case 'h':
            if (!history_file_name && optarg)
                history_file_name = strdup(optarg);
            else {
                print_usage();
                end
            }
            break;
        case 'f':
            if(!command_file_name && optarg)
                command_file_name = strdup(optarg);
            else {
                print_usage();
                end
            }
            break;
        default:
            print_usage();
            end
        }
    }

    // CREATE/OPEN HISTORY
    if (history_file_name) {
        if(access(history_file_name, R_OK|W_OK) != -1) { 
            history           = Log_create_from_file(history_file_name);
            history_file_path = get_full_path(history_file_name);
        } else {
            print_history_file_error();
            history = Log_create();
        }
    } else {
        history = Log_create();
    }
 
    if (!history) {
        end
    }

    pid_t pid = getpid();

    // COMMANDS
    if (command_file_name) {
        if(access(command_file_name, R_OK) != -1) { 
            commands = Log_create_from_file(command_file_name);

            const char *command;

            char cwd[1024];
            for (size_t iter = 0; iter < Log_size(commands); iter++) {
                command = Log_get_command(commands, iter);
                
                if (getcwd(cwd, sizeof(cwd)) != NULL && command) {
                    print_prompt(cwd, pid);
                    printf("%s\n", command);
                    parse_command(command);
                }
            }

        } else {
            print_script_file_error();
        }
        end
    } 

    // NO COMMANDS
    //SPIN FOR MORE INPUT
    char cwd[1024];
    char line[1024];
    char *pos;

    while (1) {
        if (getcwd(cwd, sizeof(cwd)) != NULL)
            print_prompt(cwd, pid);

        if (!fgets(line, 1024, stdin)) {
            end
        }

        if (*line == EOF) {
            end
        }
        if ((pos = strchr(line, '\n')) != NULL)
            *pos = '\0';
        parse_command(line);
    }
}