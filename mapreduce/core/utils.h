/**
 * MapReduce
 * CS 241 - Fall 2016
 */

#pragma once

#include <sys/types.h>

/**
 * Prints the proper usage.
 */
void print_mr1_usage();
void print_mr2_usage();

/**
 * Prints a nonzero exit status.
 * @param exec_name - name of executable
 * @param exit_status - returned exit code
 */
void print_nonzero_exit_status(char *exec_name, int exit_status);

/**
 * Prints the number of lines in the file.
 * @param filename - name of file
 */
void print_num_lines(char *filename);

/**
 * Returns a hash of the given string.
 */
unsigned int hashKey(char *key);

/**
 * Add to the array of additional file descriptors.
 */
void descriptors_add(int fd);

/**
 * Close all the file descriptors passed to descriptors_add();
 */
void descriptors_closeall();

/**
 * Deallocate the memory used to start the file descriptor list.
 */
void descriptors_destroy();

/**
 * splits a line by it's key and value.
 *
 * This function does not allocate memory. It's two outputs are both pointers
 * into the (now modified) line which is passed in.
 *
 * @param line - a newline terminated line, will be modified!
 * @param key - output
 * @param value - output
 *
 * @return "boolean" representing success or failure
 */
int split_key_value(char *line, char **key, char **value);
