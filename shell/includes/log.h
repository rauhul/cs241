/**
 * Machine Problem: Shell
 * CS 241 - Fall 2016
 */
#ifndef __CS241_LOG_H_
#define __CS241_LOG_H_
#include "vector.h"

typedef struct { Vector *vector; } Log;

/* Creates and returns a new Log. */
Log *Log_create();

/**
 * Creates a new Log and loads it with lines from the file named
 * `filename`.
 *
 * Note that if the file can not be opened or read for whatever reason,
 * this function returns an empty Log.
 */
Log *Log_create_from_file(const char *filename);

/**
 * Returns the number of commands in `log`.
 */
size_t Log_size(Log *log);

/**
 * Write all the commands in `log` to the file named `filename`.
 */
int Log_save(Log *log, const char *filename);

/**
 * Frees all the memory used by `log`.
 */
void Log_destroy(Log *log);

/**
 * Appends `command` to the end of `log`.
 */
void Log_add_command(Log *log, const char *command);

/**
 * Returns the command at the according index of the Log structure.
 */
const char *Log_get_command(Log *log, size_t index);

#endif
