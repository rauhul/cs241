/**
 * Mini Valgrind Lab
 * CS 241 - Fall 2016
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mini_valgrind.h"
#include "print.h"

/*
 * Print mini_valgrind leak report. DO NOT TOUCH THIS. You risk failing the
 * autograder if you do.
 */
void print_leak_info(meta_data *head, size_t total_usage, size_t total_free,
                     size_t bad_frees) {
  meta_data *leak_info;

  FILE *fp_write = fopen("result.txt", "wt");

  size_t total_leak = 0;

  if (fp_write != NULL) {
    fprintf(fp_write, "Heap report:\n\n");
    leak_info = head;
    while (leak_info != NULL) {
      total_leak += leak_info->size;
      fprintf(fp_write, "\tLeak file %s : line %zu\n", leak_info->file_name,
              leak_info->line_num);
      fprintf(fp_write, "\tLeak size : %zu bytes\n", leak_info->size);
      fprintf(fp_write, "\tLeak memory address : %p\n", leak_info + 1);
      fputs("\n", fp_write);
      leak_info = leak_info->next;
    }
    fprintf(fp_write, "\nProgram made %zu bad call(s) to free.\n", bad_frees);
    fprintf(fp_write, "\nHeap summary:\n\n");
    fprintf(fp_write,
            "\tTotal memory usage: %zu bytes, Total memory free: %zu bytes\n",
            total_usage, total_free);
    if (total_leak != 0) {
      fprintf(fp_write, "\tTotal leak: %zu bytes\n", total_leak);
    } else {
      fprintf(fp_write, "\tNo leak, all memory are freed, Congratulations!\n");
    }
    fclose(fp_write);
  }
  destroy();
}
