/**
 * Machine Problem: Password Cracker
 * CS 241 - Fall 2016
 */

#include "cracker1.h"
#include "format.h"
#include "utils.h"
#include "queue.h"
#include "thread_status.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>
#include <crypt.h>
#include <sys/types.h>

#define MAX_NUM_JOBS 64

//
/* cracker1.h */
typedef pthread_t pc_thread;

typedef struct pc_thread_job {
    char *name;
    char *hash;
    char *pass;
} pc_thread_job;

typedef struct pc_thread_io {
    size_t id;
    size_t numRecovered;
    size_t numFailed;
} pc_thread_io;

pc_thread_job *parse_stdin_line();
void create_pc_thread_pool(size_t thread_count);
void create_pc_thread_jobs(size_t thread_count);

void pc_thread_start(void *arg);

char *init_password_for_cracking(char *password);
/* cracker1.h */
//

// GLOBAL VARIABLES
queue_t      *job_pool;
pc_thread    *thread_pool;
pc_thread_io *thread_io_pool;

int start(size_t thread_count) {
    // CREATE POOLS
    job_pool        = queue_create(MAX_NUM_JOBS);
    thread_pool     = malloc(sizeof(pc_thread)    * thread_count);
    thread_io_pool  = malloc(sizeof(pc_thread_io) * thread_count);

    // POPULATE THREAD POOL AND THREAD IO
    create_pc_thread_pool(thread_count);

    // POPULATE JOB POOL
    create_pc_thread_jobs(thread_count);

    // BOOK KEEPING
    size_t numRecovered = 0;
    size_t numFailed    = 0;

    // JOIN THREADS
    for (size_t i = 0; i < thread_count; i++) {
        pthread_join(thread_pool[i], NULL);
        numRecovered += thread_io_pool[i].numRecovered;
        numFailed    += thread_io_pool[i].numFailed;
    }

    // OUTPUT BOOKING INFO
    v1_print_summary(numRecovered, numFailed);

    // CLEAN RESOURCES
    queue_destroy(job_pool);
    free(thread_pool);
    free(thread_io_pool);

    return 0;
}

// CREATE THREAD POOL
void create_pc_thread_pool(size_t thread_count) {
    for (size_t i = 0; i < thread_count; i++) {

        thread_io_pool[i].id           = i + 1;
        thread_io_pool[i].numRecovered = 0;
        thread_io_pool[i].numFailed    = 0;

        pthread_create(thread_pool + i, NULL, (void *) &pc_thread_start, (void *) (thread_io_pool + i));
    }
}

// PARSES STDIN FOR A JOB
void create_pc_thread_jobs(size_t thread_count) {
    pc_thread_job *job;
    while ( (job = parse_stdin_line()) ) {
        queue_push(job_pool, job);
    }

    while (thread_count) {
        queue_push(job_pool, NULL);
        thread_count--;
    }
}

// PARSES A SINGLE LINE FROM STDIN FOR A JOB
pc_thread_job *parse_stdin_line() {
    char   *line = NULL;
    size_t  len = 0;
    ssize_t read;

    char *name;
    char *hash;
    char *pass;

    while ((read = getline(&line, &len, stdin)) != -1) {
        line[read - 1] = '\0';

        name    = line;

        hash    = strchr(name, ' ');
        hash[0] = '\0';
        hash   += 1;

        pass    = strchr(hash, ' ');
        pass[0] = '\0';
        pass   += 1;

        pc_thread_job *job = malloc(sizeof(pc_thread_job));
        job->name = name;
        job->hash = hash;
        job->pass = pass;
        return job;
    }

    free(line);
    return NULL;
}

// WORKER ROUTINE FOR A SINGLE THREAD
void pc_thread_start(void *arg) {
    pc_thread_io *thread_io = arg;

    int end;
    int result;
    int hashCount;
    double timeElapsed;

    struct crypt_data cdata;
    cdata.initialized = 0;
    const char *hash;

    pc_thread_job *job;
    while ( (job = queue_pull(job_pool)) ) {
        end       = 0;
        result    = 0;
        hashCount = 0;
        timeElapsed = getThreadCPUTime();

        v1_print_thread_start(thread_io->id, job->name);
        char *last_test_password = init_password_for_cracking(job->pass);

        do {
            hash = crypt_r(job->pass, "xx", &cdata);
            hashCount++;
            result = !strcmp(hash, job->hash);
            end = !strcmp(last_test_password, job->pass);
        } while (!end && !result && incrementString(job->pass));

        if (result) {
            thread_io->numRecovered++;
        } else {
            thread_io->numFailed++;
        }

        v1_print_thread_result(thread_io->id, job->name, job->pass, hashCount, getThreadCPUTime() - timeElapsed, !result);

        free(last_test_password);
        free(job->name);
        free(job);
    }
}

// SET UP PASSWORD FOR CRACKING
char *init_password_for_cracking(char *password) {
    char *last_test_password = strdup(password);
    char *last_test_password_iter = last_test_password;
    while (*password) {
        if (*password == '.') {
            *password = 'a';
            *last_test_password_iter = 'z';
        }
        password++;
        last_test_password_iter++;
    }
    return last_test_password;
}













