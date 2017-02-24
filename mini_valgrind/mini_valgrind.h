/**
 * Mini Valgrind Lab
 * CS 241 - Fall 2016
 */

#ifndef MINI_VALGRIND_H
#define MINI_VALGRIND_H

#include <stddef.h>
#include <stdlib.h>

#define MAX_FILENAME_LENGTH 128
#define malloc(size) mini_malloc(size, __FILE__, __LINE__)
#define free(ptr) mini_free(ptr)
#define realloc(ptr, size) mini_realloc(ptr, size, __FILE__, __LINE__)

typedef struct _meta_data {

  // size of the memory allocated by malloc
  size_t size;
  size_t line_num;
  char file_name[MAX_FILENAME_LENGTH];
  struct _meta_data *next;
} meta_data;

meta_data *head;
/*
 * The total usage should calculate the total memory usage, but not including
 * the memory for
 * meta data.
 * The total free should calculate the total memory that has been free, still
 * not including
 * the memory for meta data.
 * The bad frees count how many invalid frees have been made.
 * The unit for all these three variales are bytes.
 */
size_t total_usage;
size_t total_free;
size_t bad_frees;

extern void destroy();

void *mini_malloc(size_t size, const char *file, size_t line);
void *mini_realloc(void *ptr, size_t size, const char *file, size_t line);
void mini_free(void *ptr);

void insert_meta_data(meta_data *md, size_t size, const char *file,
                      size_t line);
void remove_meta_data(void *ptr);
extern void print_report();

#endif
