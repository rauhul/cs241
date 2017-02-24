/**
 * Machine Problem: Password Cracker
 * CS 241 - Fall 2016
 */

#ifndef __THREAD_STATUS_H__
#define __THREAD_STATUS_H__

/**
 * This provides a lightweight method for threads to report what they're
 * currently doing.
 */

#include <pthread.h>
#include <semaphore.h>

/**
 * Sets the file descriptor to which threadStatsPrint will write.  It it set to
 * STDERR_FILENO by default.
 * Set it to a negative value to disable the output.
 */
void threadStatusSetOutput(int fd);

/**
 * Prints the status of all the threads. This may be used as a signal handler.
 */
void threadStatusPrint(int signo_ignored);

/**
 * Sets the status of a thread.
 *
 * description: a string describing what the thread is doing.
 * This should be a constant string literal.  This pointer may be
 * kept for an arbitrary length of time before use, and it will not
 * be freed.
 */
#define threadStatusSet(description)                                           \
  threadStatusSetInternal((description), __FILE__, __LINE__)

void threadStatusSetInternal(const char *description, const char *src_filename,
                             int src_line);

/**
 * The macros below wrap thread synchronization function that may block
 * with a call to set the thread status, so you can easily see where
 * each thread is blocked.
 *
 * Define DISABLE_THREAD_STATUS_OVERRIDES if you want to disable
 * this functionality.
 */

#ifndef DISABLE_THREAD_STATUS_OVERRIDES

#define pthread_create(t, attr, fn, arg)                                       \
  ts_pthread_create((t), (attr), (fn), (arg))

#define pthread_mutex_lock(m) ts_pthread_mutex_lock((m), __FILE__, __LINE__)

#define pthread_mutex_timedlock(m, a)                                          \
  ts_pthread_mutex_timedlock((m), (a), __FILE__, __LINE__)

#define pthread_cond_wait(c, m)                                                \
  ts_pthread_cond_wait((c), (m), __FILE__, __LINE__)

#define pthread_cond_timedwait(c, m, a)                                        \
  ts_pthread_cond_timedwait((c), (m), (a), __FILE__, __LINE__)

#define pthread_join(t, val) ts_pthread_join((t), (val), __FILE__, __LINE__)

#define sem_wait(s) ts_sem_wait((s), __FILE__, __LINE__)

#define pthread_barrier_wait(b) ts_pthread_barrier_wait((b), __FILE__, __LINE__)

int ts_pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                      void *(*start_routine)(void *), void *arg);
int ts_pthread_mutex_lock(pthread_mutex_t *mutex, const char *src_filename,
                          int src_line);
int ts_pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex,
                         const char *src_filename, int src_line);
int ts_pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex,
                              const struct timespec *abstime,
                              const char *src_filename, int src_line);
int ts_pthread_join(pthread_t thread, void **value_ptr,
                    const char *src_filename, int src_line);
int ts_sem_wait(sem_t *sem, const char *src_filename, int src_line);

// #ifdef __USE_XOPEN2K
int ts_pthread_mutex_timedlock(pthread_mutex_t *mutex,
                               const struct timespec *abstime,
                               const char *src_filename, int src_line);
int ts_pthread_barrier_wait(pthread_barrier_t *barrier,
                            const char *src_filename, int src_line);
// #endif  // open2k

#endif /* disable overrides */

#endif /* __THREAD_STATUS_H__ */
