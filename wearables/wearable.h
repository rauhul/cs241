/**
 * Machine Problem: Wearables
 * CS 241 - Fall 2016
 */

#pragma once

#include <sys/types.h>
#include <unistd.h>

#include "vector.h"

/**
 * Types of data
 */
static const char *TYPE1 = "heart_beat";
static const char *TYPE2 = "blood_sugar";
static const char *TYPE3 = "body_temp";

/**
 * Holds a data type and value
 */
typedef struct SampleData {
  char *type_;
  int data_;
} SampleData;

/**
 * Holds the information for a wearable
 */
typedef struct Wearable {
  // The port we will connect on
  const char *wearable_port_;
  // start offset (in millis) that this wearable will start to broadcast
  useconds_t start_sleep_;
  // the time (in millis) between transmittions
  useconds_t sample_interval_;
  // the total amount of data points
  size_t data_size_;
  SampleData *data_;
  Vector *results_;
} Wearable;

/**
 * Loads all wearables from a file.
 * Format is for a wearable is
 *
 * BEGIN
 * START:<time until start (in millis)>
 * INTERVAL:<time between intervals (in millis)>
 * <type>:<transmit_variables>
 * ...
 * END
 */
Wearable **get_wearables(const char *file_name, size_t *wearable_count,
                         int **sample_times, size_t *sample_count,
                         useconds_t *latest_launch, const char *wearable_port);

void free_wearable(Wearable *wearable);

/**
 * Runs one wearable. Takes a pointer to a Wearable as an argument.
 */
void *wearable_thread(void *param);
