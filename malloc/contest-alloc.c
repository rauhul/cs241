/**
 * Machine Problem: Malloc
 * CS 241 - Fall 2016
 */

#include "contest.h"
#include <dlfcn.h>
#include <fcntl.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static void *alloc_handle = NULL;

static void *(*alloc_calloc)(size_t nmemb, size_t size) = NULL;
static void *(*alloc_malloc)(size_t size) = NULL;
static void (*alloc_free)(void *ptr) = NULL;
static void *(*alloc_realloc)(void *ptr, size_t size) = NULL;

static void *(*libc_calloc)(size_t nmemb, size_t size) = NULL;
static void *(*libc_malloc)(size_t size) = NULL;
static void (*libc_free)(void *ptr) = NULL;
static void *(*libc_realloc)(void *ptr, size_t size) = NULL;
static void *(*libc_sbrk)(size_t size) = NULL;

static void *sbrk_start = NULL;
static void *sbrk_end = NULL;
static void *sbrk_current = NULL;
static void *sbrk_largest = NULL;
static void *sbrk_init_done = NULL;
static size_t sbrk_counter = 0;

static alloc_stats_t *stats = NULL;

static int inside_init = 0;

/*
 * The replacement sbrk. Naming this the same as sbrk() overrides the actual
 * sbrk().
 * We verify that the return pointers from malloc/realloc return with the heap's
 * bounds.
 */
void *sbrk(intptr_t size) {
  sbrk_counter += 1;

  void *sbrk_return = libc_sbrk(size);
  sbrk_current = libc_sbrk(0);
  return sbrk_return;
}

/*
 * Ensure that any memory returned by malloc is actually, really allocated
 * within
 * the bounds of the heap.
 */
void verify_address(void *ptr) {
  if (ptr < sbrk_start || ptr >= sbrk_end) {
    fprintf(stderr, "Data allocated outside of heap.");
    exit(91);
  }
}

static void contest_alloc_init() {
  inside_init = 1;

  /* Tell malloc() not to use mmap() */
  mallopt(M_MMAP_MAX, 0);

  libc_calloc = dlsym(RTLD_NEXT, "calloc");
  libc_malloc = dlsym(RTLD_NEXT, "malloc");
  libc_free = dlsym(RTLD_NEXT, "free");
  libc_realloc = dlsym(RTLD_NEXT, "realloc");
  libc_sbrk = dlsym(RTLD_NEXT, "sbrk");

  sbrk_start = sbrk_largest = sbrk_current = sbrk(0);
  sbrk_end = (char *)sbrk_start + MEMORY_LIMIT;

  inside_init = 2;

  alloc_handle = dlopen("./alloc.so", RTLD_NOW | RTLD_GLOBAL);
  if (!alloc_handle) {
    char *err = dlerror();

    if (err)
      fprintf(stderr, "A dynamic linking error occurred: (%s)\n", err);
    else
      fprintf(stderr, "An unknown dynamic linking error occurred.\n");

    exit(65);
  }

  alloc_calloc = dlsym(alloc_handle, "calloc");
  alloc_malloc = dlsym(alloc_handle, "malloc");
  alloc_free = dlsym(alloc_handle, "free");
  alloc_realloc = dlsym(alloc_handle, "realloc");

  if (!alloc_calloc || !alloc_malloc || !alloc_free || !alloc_realloc) {
    fprintf(stderr,
            "Unable to dynamicly load a required memory allocation call.\n");
    exit(66);
  }

  char *file_name = getenv("ALLOC_CONTEST_MMAP");
  int fd = open(file_name, O_RDWR);
  stats = mmap(NULL, sizeof(alloc_stats_t), PROT_WRITE, MAP_SHARED, fd, 0);

  if (fd <= 0 || stats == (void *)-1) {
    fprintf(stderr, "fd/mmap");
    exit(67);
  }

  stats->max_heap_used = 0;
  stats->memory_heap_sum = 0;
  stats->memory_uses = 0;

  sbrk_init_done = sbrk(0);
  inside_init = 0;
}

static void contest_tracking() {
  sbrk_current = libc_sbrk(0);
  unsigned long current_mem_usage = ((long)sbrk_current - (long)sbrk_init_done);

  if (stats->max_heap_used < current_mem_usage) {
    sbrk_largest = sbrk_current;
    stats->max_heap_used = current_mem_usage;

    if (current_mem_usage > MEMORY_LIMIT) {
      fprintf(stderr, "Exceeded Memory Limit (%lu)\n", MEMORY_LIMIT);
      exit(68);
    }
  }

  stats->memory_heap_sum += current_mem_usage;
  stats->memory_uses++;
}

void *calloc(size_t nmemb, size_t size) {
  if (inside_init) {
    if (inside_init == 1) {
      void *ptr = sbrk(nmemb * size);
      memset(ptr, 0x00, nmemb * size);
      return ptr;
    } else
      return libc_calloc(nmemb, size);
  }

  if (!alloc_handle)
    contest_alloc_init();

  void *addr = alloc_calloc(nmemb, size);
  verify_address(addr);
  contest_tracking();

  return addr;
}

void *malloc(size_t size) {
  if (inside_init)
    return libc_malloc(size);

  if (!alloc_handle)
    contest_alloc_init();

  void *addr = alloc_malloc(size);
  verify_address(addr);
  contest_tracking();

  return addr;
}

void free(void *ptr) {
  if (inside_init) {
    libc_free(ptr);
    return;
  }

  if (!alloc_handle)
    contest_alloc_init();

  if (ptr < sbrk_init_done) {
    libc_free(ptr);
    return;
  }

  if (ptr) {
    alloc_free(ptr);
    contest_tracking();
  }
}

void *realloc(void *ptr, size_t size) {
  if (inside_init)
    return libc_realloc(ptr, size);

  if (!alloc_handle)
    contest_alloc_init();

  void *addr;
  if (!ptr) {
    addr = alloc_malloc(size);
    verify_address(addr);
  } else if (size == 0) {
    alloc_free(ptr);
    addr = NULL;
  } else {
    addr = alloc_realloc(ptr, size);
    verify_address(addr);
  }
  contest_tracking();

  return addr;
}
