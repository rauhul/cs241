/**
 * Machine Problem: Wearables
 * CS 241 - Fall 2016
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "wearable.h"

typedef uint64_t timestamp;

typedef struct timestamp_entry {
  timestamp time;
  void *data;
} timestamp_entry;

/**
 * Copy constructor for timestamp_entry, for use in Vector.
 */
void *timestamp_entry_copy_constructor(void *elem);

/**
 * Destructor for timestamp_entry, for use in Vector.
 */
void timestamp_entry_destructor(void *elem);

/**
 * Selector type for `Vector_gather`. Returns true if the given
 * `timestamp_entry` should be included in the output, and false otherwise.
 */
typedef bool (*selector)(timestamp_entry *);

/**
 * Selects a certain ordered subset of the entries in the `vector`, ordered by
 * timestamp. The `vector` must contain elements of type `timestamp_entry`.
 * The output array `output_timestamps` will contain only those timestamps in
 * `vector` between `start` and `end` for which `predicate` returns true, and
 * will be ordered by timestamp. `output_timestamps` will be set to a
 * heap-allocated array. The caller must free output_timestamps. Returns the
 * length of `output_timestamps`, or -1 if a failure occured.
 */
ssize_t gather_timestamps(Vector *vector, timestamp start, timestamp end,
                          selector predicate,
                          timestamp_entry **output_timestamps);

/**
 * Given an input line in the form <timestamp>:<value>:<type>, this method
 * parses the infomration from the string, into the given timestamp, and mallocs
 * space for SampleData, and stores the type and value within
 */
void extract_key(char *line, long *timestamp, SampleData **ret);

/**
 * Used to write out the statistics of a given array of timestamp_entry. To
 * generate the result array see gather_timestamps(). The data is written to the
 * file descriptor `fd` and will write the first `size` elements of `resuts`.
 * Note that for a single request this function should be called once for each
 * type and then "\r\n" should be written to signifiy the end of the message.
 */
void write_results(int fd, const char *type, timestamp_entry *results,
                   int size);
