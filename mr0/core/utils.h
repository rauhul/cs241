/**
 * Map Reduce 0 Lab
 * CS 241 - Fall 2016
 */
#ifndef UTILS_H
#define UTILS_H

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void print_usage();

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
 * Prints the number of lines in the file.
 * @param filename - name of file
 */
void print_num_lines(char *filename);

#endif
