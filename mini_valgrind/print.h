/**
 * Mini Valgrind Lab
 * CS 241 - Fall 2016
 */

#ifndef PRINT_H
#define PRINT_H
#include "mini_valgrind.h"

extern void print_leak_info(meta_data *head, size_t total_usage,
                            size_t total_free, size_t bad_frees);

#endif
