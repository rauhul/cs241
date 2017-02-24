/**
 * Machine Problem: Wearables
 * CS 241 - Fall 2016
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "utils.h"
#include "vector.h"
#include "wearable.h"

void *timestamp_entry_copy_constructor(void *elem) {
  timestamp_entry *_elem = (timestamp_entry *)elem;
  timestamp_entry *out = malloc(sizeof(timestamp_entry));
  out->time = _elem->time;
  out->data = _elem->data;
  return out;
}

void timestamp_entry_destructor(void *elem) { free(elem); }

static int compare_timestamp(const void *v1, const void *v2) {
  return ((timestamp_entry *)v1)->time < ((timestamp_entry *)v2)->time;
}

ssize_t gather_timestamps(Vector *vector, timestamp start, timestamp end,
                          selector predicate,
                          timestamp_entry **output_timestamps) {
  if (output_timestamps == NULL || predicate == NULL) {
    return -1;
  }

  ssize_t return_size = 0;
  *output_timestamps = NULL;
  for (size_t i = 0; i < Vector_size(vector); i++) {
    timestamp_entry *entry = Vector_get(vector, i);
    if (entry->time >= start && entry->time < end && predicate(entry)) {
      return_size += 1;
      *output_timestamps =
          realloc(*output_timestamps, return_size * sizeof(timestamp_entry));
      (*output_timestamps)[return_size - 1] = *entry;
    }
  }

  qsort(*output_timestamps, return_size, sizeof(timestamp_entry),
        compare_timestamp);
  return return_size;
}

int compare(const void *a, const void *b) { return (*(int *)a - *(int *)b); }

void write_results(int fd, const char *type, timestamp_entry *results,
                   int size) {
  long avg = 0;
  int i;

  char buffer[1024];
  int temp_array[size];
  sprintf(buffer, "Results for %s:\n", type);
  sprintf(buffer + strlen(buffer), "Size:%i\n", size);
  for (i = 0; i < size; i++) {
    temp_array[i] = ((SampleData *)(results[i].data))->data_;
    avg += ((SampleData *)(results[i].data))->data_;
  }

  qsort(temp_array, size, sizeof(int), compare);

  if (size != 0) {
    sprintf(buffer + strlen(buffer), "Median:%i\n",
            (size % 2 == 0)
                ? (temp_array[size / 2] + temp_array[size / 2 - 1]) / 2
                : temp_array[size / 2]);
  } else {
    sprintf(buffer + strlen(buffer), "Median:0\n");
  }

  sprintf(buffer + strlen(buffer), "Average:%li\n\n",
          (size == 0 ? 0 : avg / size));
  write(fd, buffer, strlen(buffer));
}

void extract_key(char *line, long *timestamp, SampleData **ret) {
  *ret = malloc(sizeof(SampleData));
  sscanf(line, "%zu:%i:%ms\n", timestamp, &((*ret)->data_), &(*ret)->type_);
  // eat the trailing ":"
  (*ret)->type_[strlen((*ret)->type_) - 1] = '\0';
}
