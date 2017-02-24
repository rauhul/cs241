/**
 * Machine Problem: Malloc
 * CS 241 - Fall 2016
 */

#ifndef _CONTEST_H_
#define _CONTEST_H_

typedef struct _alloc_stats_t {
  unsigned long long max_heap_used;
  unsigned long memory_uses;
  unsigned long long memory_heap_sum;
} alloc_stats_t;

#define MEMORY_LIMIT ((1024L * 1024L * 1024L * 2L) + (1024L * 1024L * 512L))

// timeout in seconds
#define TIMER_TIMEOUT 30

#endif
