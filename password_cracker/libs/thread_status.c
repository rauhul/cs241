/**
 * Machine Problem: Password Cracker
 * CS 241 - Fall 2016
 */

#include <assert.h>
#include <limits.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define DISABLE_THREAD_STATUS_OVERRIDES
#include "thread_status.h"

#define INIT_ARRAY_SIZE 20

#ifdef SANITIZE_THREADS
#define LOCK_IF_SANITIZED pthread_mutex_lock(&mutex)
#define UNLOCK_IF_SANITIZED pthread_mutex_unlock(&mutex)
#else
#define LOCK_IF_SANITIZED                                                      \
  do {                                                                         \
  } while (0)
#define UNLOCK_IF_SANITIZED                                                    \
  do {                                                                         \
  } while (0)
#endif

// thread_status and src_filename just hold references to the strings;
// to keep things lightweight they don't copy the strings.
typedef struct {
  pthread_t thread_id;
  const char *thread_status;
  const char *src_filename;
  int src_line;
} ThreadStatus;

// guard the following three variables
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static int thread_count = 1;
static ThreadStatus *status_array = NULL;
static int status_array_cap = 0;

// file descriptor to which output will be written
// Set it to a negative value to disable output.
static int thread_status_fd = STDERR_FILENO;

static void initThreadArray();
static int findThreadId(pthread_t thread_id);

// Deallocate status_array
static void threadStatusDestroy();

static void writeStr(int fd, const char *str);
static void writeInt(int fd, int x);

// Deallocate the data
static void threadStatusDestroy() {
  if (status_array)
    free(status_array);
}

// Sets the file descriptor to which threadStatsPrint will write.
// Set it to a negative value to disable the output.
void threadStatusSetOutput(int fd) { thread_status_fd = fd; }

// Write a string to a file descriptor
static void writeStr(int fd, const char *str) {
  size_t len = strlen(str);
  ssize_t __attribute__((unused)) res = write(fd, str, len);
}

/* Convert an int to a string and write it to the given file
   descriptor without calling any functions that shouldn't be called
   from a signal handler. */
static void writeInt(int fd, int sx) {
  assert(sizeof(sx) == 4);

  if (sx == 0) {
    // because this algorithm doesn't handle zero
    writeStr(fd, "0");
    return;
  }

  unsigned x;
  int is_negative = 0, buf_len = 0, i;
  char buf[12];

  if (sx < 0) {
    x = -sx;
    is_negative = 1;
  } else {
    x = sx;
  }

  // write the digits, least-significant first
  while (x) {
    buf[buf_len++] = '0' + (x % 10);
    x /= 10;
  }
  if (is_negative)
    buf[buf_len++] = '-';
  buf[buf_len] = 0;

  // -1234 has been written as 4321-
  // now reverse the digits;

  for (i = (buf_len >> 1) - 1; i >= 0; i--) {
    int tmp = buf[i];
    buf[i] = buf[buf_len - i - 1];
    buf[buf_len - i - 1] = tmp;
  }

  ssize_t __attribute__((unused)) res = write(fd, buf, buf_len);
}

void threadStatusPrint(__attribute__((unused)) int signo_ignored) {
  // minimize the typing
  int fd = thread_status_fd;
  if (fd < 0)
    return;

  LOCK_IF_SANITIZED;

  if (!status_array) {
    writeStr(fd, "Thread status array not initialized.\n");
  } else {
    int i;
    writeStr(fd, "\n");
    for (i = 0; i < thread_count; i++) {
      writeStr(fd, "** Thread ");
      writeInt(fd, i);
      writeStr(fd, ": ");
      writeStr(fd, (const char *)status_array[i].thread_status);
      writeStr(fd, " at ");
      writeStr(fd, (const char *)status_array[i].src_filename);
      writeStr(fd, ":");
      writeInt(fd, status_array[i].src_line);
      writeStr(fd, "\n");
    }
    writeStr(fd, "\n");
  }

  UNLOCK_IF_SANITIZED;
}

/* Do the work of threadStatusSet, copying the given status, but also
   save the __FILE__ and __LINE__ variables from the macro. */
void threadStatusSetInternal(const char *description, const char *src_filename,
                             int src_line) {
  int thread_id = findThreadId(0);
  if (thread_id == -1) {
    fprintf(stderr, "findThreadId failed on %lu\n", pthread_self());
    return;
  }

  LOCK_IF_SANITIZED;
  assert(status_array);
  assert(thread_id >= 0 && thread_id < thread_count);
  if (description == NULL)
    description = "(null)";
  if (src_filename == NULL)
    src_filename = "(null)";

  status_array[thread_id].thread_id = pthread_self();
  status_array[thread_id].thread_status = description;
  status_array[thread_id].src_filename = src_filename;
  status_array[thread_id].src_line = src_line;
  UNLOCK_IF_SANITIZED;
}

static void initThread(int i, pthread_t thread_id) {
  assert(status_array);
  assert(thread_count > i);
  status_array[i].thread_id = thread_id;
  status_array[i].thread_status = "(none)";
  status_array[i].src_filename = "(none)";
  status_array[i].src_line = 0;
}

static void initThreadArray() {
  if (status_array == NULL) {

    // if the status array hasn't been initialized, there should only
    // be one thread (no calls to pthread_create() yet)
    assert(thread_count == 1);

    status_array_cap = INIT_ARRAY_SIZE;
    status_array =
        (ThreadStatus *)malloc(sizeof(ThreadStatus) * status_array_cap);

    // save the id of the main thread
    initThread(0, pthread_self());

    // printf("Add thread %lu as %d\n", (long unsigned)pthread_self(), 0);

    atexit(threadStatusDestroy);
  }
}

/* Add a new thread (should only be called from findThreadId() */
static int addThread(pthread_t new_thread) {

  // grow the array if necessary
  if (status_array_cap == thread_count) {
    status_array_cap *= 2;
    status_array = (ThreadStatus *)realloc(status_array, sizeof(ThreadStatus) *
                                                             status_array_cap);
  }
  int idx = thread_count++;
  initThread(idx, new_thread);

  // printf("Add thread %lu as %d\n", (long unsigned)new_thread, idx);

  return idx;
}

/* Look up the 0..(n-1) integer index for this thread based on the
   pthread_t thid ids saved in the status array. If its not found, add it. */
static int findThreadId(pthread_t thread_id) {
  int idx = -1;

  // this is the one mutex that is required
  pthread_mutex_lock(&mutex);
  initThreadArray();

  if (thread_id == 0) {
    thread_id = pthread_self();
  }

  int i;
  for (i = 0; i < thread_count; i++) {
    if (status_array[i].thread_id == thread_id) {
      idx = i;
      break;
    }
  }

  // not found; add it
  if (idx == -1) {
    idx = addThread(thread_id);
    // fprintf(stderr, "Auto-add %ld as %d\n", thread_id, idx);
  }

  pthread_mutex_unlock(&mutex);

  return idx;
}

/* save a copy of the current status so it can be restored later */
static int saveStatus(ThreadStatus *saved, const char *status,
                      const char *src_filename, int src_line) {
  int tid = findThreadId(0);

  LOCK_IF_SANITIZED;
  if (tid != -1) {
    *saved = status_array[tid];
    status_array[tid].thread_status = status;
    status_array[tid].src_filename = src_filename;
    status_array[tid].src_line = src_line;
  }
  UNLOCK_IF_SANITIZED;

  return tid;
}

static void restoreStatus(int tid, ThreadStatus saved) {
  LOCK_IF_SANITIZED;
  status_array[tid] = saved;
  UNLOCK_IF_SANITIZED;
}

/* The user is creating another thread; capture the thread id. */
int ts_pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                      void *(*start_routine)(void *), void *arg) {

  pthread_t child_thread_id;
  int result;

  // make sure the thread array is initialized
  initThreadArray();

  result = pthread_create(&child_thread_id, attr, start_routine, arg);
  if (result == 0) {
    // call findThreadId to make sure this thread is added to the array
    findThreadId(child_thread_id);
  }

  if (thread)
    *thread = child_thread_id;

  return result;
}

/* provide wrappers for a few synchronization functions that may block */

int ts_pthread_mutex_lock(pthread_mutex_t *mutex, const char *src_filename,
                          int src_line) {
  ThreadStatus saved;
  int tid = saveStatus(&saved, "mutex lock", src_filename, src_line);
  int result = pthread_mutex_lock(mutex);
  if (tid >= 0)
    restoreStatus(tid, saved);
  return result;
}

int ts_pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex,
                         const char *src_filename, int src_line) {
  ThreadStatus saved;
  int tid = saveStatus(&saved, "condvar wait", src_filename, src_line);
  int result = pthread_cond_wait(cond, mutex);
  if (tid >= 0)
    restoreStatus(tid, saved);
  return result;
}

int ts_pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex,
                              const struct timespec *abstime,
                              const char *src_filename, int src_line) {
  ThreadStatus saved;
  int tid = saveStatus(&saved, "condvar timed wait", src_filename, src_line);
  int result = pthread_cond_timedwait(cond, mutex, abstime);
  if (tid >= 0)
    restoreStatus(tid, saved);
  return result;
}

int ts_pthread_join(pthread_t thread, void **value_ptr,
                    const char *src_filename, int src_line) {
  ThreadStatus saved;
  int tid = saveStatus(&saved, "pthread join", src_filename, src_line);
  int result = pthread_join(thread, value_ptr);
  if (tid >= 0)
    restoreStatus(tid, saved);
  return result;
}

int ts_sem_wait(sem_t *sem, const char *src_filename, int src_line) {
  ThreadStatus saved;
  int tid = saveStatus(&saved, "semaphore wait", src_filename, src_line);
  int result = sem_wait(sem);
  if (tid >= 0)
    restoreStatus(tid, saved);
  return result;
}

int ts_pthread_mutex_timedlock(pthread_mutex_t *mutex,
                               const struct timespec *abstime,
                               const char *src_filename, int src_line) {
  ThreadStatus saved;
  int tid = saveStatus(&saved, "mutex timed lock", src_filename, src_line);
  int result = pthread_mutex_timedlock(mutex, abstime);
  if (tid >= 0)
    restoreStatus(tid, saved);
  return result;
}

int ts_pthread_barrier_wait(pthread_barrier_t *barrier,
                            const char *src_filename, int src_line) {
  ThreadStatus saved;
  int tid = saveStatus(&saved, "barrier wait", src_filename, src_line);
  int result = pthread_barrier_wait(barrier);
  if (tid >= 0)
    restoreStatus(tid, saved);
  return result;
}
