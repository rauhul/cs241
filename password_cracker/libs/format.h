/**
 * Machine Problem: Password Cracker
 * CS 241 - Fall 2016
 */

#ifndef FORMAT_H_
#define FORMAT_H_
/**
 * format.h - This library contains functions that you should use when output
 * is needed.
 *
 * IMPORTANT: We will only grade output coming out of this library.
 */

/**
 * Prints out a start message as a thread begins to process a task
 *
 * @threadId: the id of the worker thread
 * @username: the username of the task the thread is working on
 *
 * THIS FUNCTION SHOULD ONLY BE USED IN VERSION 1 OF THIS MP
 */
void v1_print_thread_start(int threadId, char *username);

/**
 * Prints out a thread's cracking result
 *
 * @threadId: the id of the worker thread
 * @username: the username of the task the thread is working on
 * @password: the password that has been cracked
 * @hashCount: the number of hashes it has computed
 * @timeElapsed: time in seconds the thread has taken to crack the password
 * @result: a value of 0 indicates a successful crack and a value of 1 means it
 *			has failed
 *
 * THIS FUNCTION SHOULD ONLY BE USED IN VERSION 1 OF THIS MP
 */
void v1_print_thread_result(int threadId, char *username, char *password,
                            int hashCount, double timeElapsed, int result);

/**
 * Prints a summary of the cracking process
 * @numRecovered: the number of successful attempts
 * @numFailed: the number of failed attempts
 *
 * THIS FUNCTION SHOULD ONLY BE USED IN VERSION 1 OF THIS MP
 */
void v1_print_summary(int numRecovered, int numFailed);

/**
 * Prints the user whose password will be cracked
 *
 * @username: the username of the task the thread is working on
 *
 * THIS FUNCTION SHOULD ONLY BE USED IN VERSION 2 OF THIS MP
 */
void v2_print_start_user(char *username);

/**
 * Prints out a start message as a thread begins to process a task
 *
 * @threadId: the id of the worker thread
 * @username: the username of the task the thread is working on
 * @offset: the offset from which this thread will work on
 * @startPassword: the password that the thread will start cracking from
 *
 * THIS FUNCTION SHOULD ONLY BE USED IN VERSION 2 OF THIS MP
 */
void v2_print_thread_start(int threadId, char *username, long offset,
                           char *startPassword);

/**
 * Prints out a thread's cracking result
 *
 * @threadId: the id of the worker thread
 * @hashCount: the number of hashes it has computed
 * @result: 0 - the thread has successfully cracked the password
 * 			1 - the thread was stopped early because the password was
 *cracked by
 *				another thread
 *			2 - the thread has finished searching in the range it was
 *given and
 *				failed to find the password
 *
 * THIS FUNCTION SHOULD ONLY BE USED IN VERSION 2 OF THIS MP
 */
void v2_print_thread_result(int threadId, int hashCount, int result);

/**
 * Print the cracking result of a specific cracking task
 *
 * @username: the username of the task
 * @password: the password for the user if found
 * @hashCount: the number of hashes computed
 * @timeElapsed: total time elapsed
 * @totalCPUTime: total CPU time used
 * @result: a value of 0 indicates a successful crack and a value of 1 means it
 *			has failed
 *
 * * THIS FUNCTION SHOULD ONLY BE USED IN VERSION 2 OF THIS MP
 */
void v2_print_summary(char *username, char *password, int hashCount,
                      double elapsedTime, double totalCPUTime, int result);

#endif
