/**
 * Machine Problem: Password Cracker
 * CS 241 - Fall 2016
 */

#ifndef CRACKER_UTILS_H
#define CRACKER_UTILS_H

#include <pthread.h>

/**
 * Returns a "wall clock" timer value in seconds.
 * This timer keeps ticking even when the thread or process is idle.
 */
double getTime();

/**
 * Returns a process-wide CPU time value in seconds.
 * This will tick faster than getTime() if mutiple threads are busy.
 */
double getCPUTime();

/**
 * Return a thread-local CPU time value in seconds.
 * This timer only ticks when the current thread is not idle.
 */
double getThreadCPUTime();

/**
 * Given a string like "abc.....", return the number of letters in it.
 */
int getPrefixLength(const char *str);

/**
 * Set 'result' to the string that you'd get after replacing every character
 * with 'a' and calling incrementString() on it 'n' times.
 */
void setStringPosition(char *result, long n);

/**
 * Increment the letters in 'str'.  Returns 1 if the increment is successful.
 * Returns 0 if all the letters in the string are 'z'.
 *
 * For example:
 *   If str=="howdy", str will be changed to "howdz"
 *     and the function will return 1.
 *
 *   If str=="jazz", str will be changed to "jbaa"
 *     and the function will return 1.
 *
 *   If str=="zzzzzz", str will be unchanged and the
 *     function will return 0.
 *
 * 'str' must contain only lowercase letters, and it will contain only lowercase
 * letters when the function returns.
 */
int incrementString(char *str);

/* Compute the subrange on which each thread should work.
 * unknown_letter_count - The number of letters in the password that are not
 * known.
 * thread_count - The number of worker threads.
 * thread_id - My thread ID, where the IDs start at 1.
 * start_index - (output parameter) This will be set to the index of
 * the first possible password this thread should try.
 * count - (output parameter) This will be set to the number of
 * passwords this thread should try.
 */
void getSubrange(int unknown_letter_count, size_t thread_count, int thread_id,
                 long *start_index, long *count);

#endif
