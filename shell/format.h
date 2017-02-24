/**
 * Machine Problem: Shell
 * CS 241 - Fall 2016
 */
#ifndef __CS241_FORMAT_H__
#define __CS241_FORMAT_H__
#include <stdio.h>
#include <stdio.h>
#include <sys/types.h>

/**
 * This library will handle all the formatting
 * for your shell.
 *
 * Please use this to ensure your formatting
 * matches what the autograder expects.
 */

/**
 * Before the first prompt is printed, print your netid.
 */
void print_shell_owner(const char *netid);

/**
 * Print the correct usage on invalid command line arguments.
 */
void print_usage();

/**
 * Print a command and newline.
 */
void print_command(const char *command);

/**
 * Print when the user specifies a non-existent script file.
 */
void print_script_file_error();

/**
 * Print when the user specifies a non-existent history file.
 */
void print_history_file_error();

/**
 * Print the shell prompt with current directory `directory` and current
 * process id `pid`.
 */
void print_prompt(const char *directory, pid_t pid);

/**
 * Print a single line of history. Both `index` and `command` are the same as
 * in the history log.
 */
void print_history_line(size_t index, const char *command);

/**
 * Print when the user specifies and out-of-bounds history command.
 */
void print_invalid_index();

/**
 * Print when the history search fails (no command has the prefix).
 */
void print_no_history_match();

/**
 * Print when the user tries to change directories but `path` does not exist.
 */
void print_no_directory(const char *path);

/**
 * Print when executing an external command with a process id `pid`.
 */
void print_command_executed(pid_t pid);

/**
 * Print when fork fails.
 */
void print_fork_failed();

/**
 * Print when exec fails.
 */
void print_exec_failed(const char *command);

/**
 * Print when wait fails.
 */
void print_wait_failed();

/**
 * Splits a copy of 'str' by a 'delim'.
 * Then writes the number of tokens into 'numtokens'.
 * These strings are allocated on the heap, so call free_args(char **args) when
 * you are done with them.
 * Learn more at:
 * https://www.quora.com/How-do-you-write-a-C-program-to-split-a-string-by-a-delimiter
 */
char **strsplit(const char *str, const char *delim, size_t *numtokens);

/**
 * Frees the memory returned by strsplit().
 */
void free_args(char **args);

/**
 * Returns the full path a file named 'filename' in the current directory.
 * The path is stored on the heap, so it is up the caller to free it.
 */
char *get_full_path(char *filename);

#endif
