/**
 * Machine Problem: Password Cracker
 * CS 241 - Fall 2016
 */

/* local files */
#include "cracker2.h"
#include "format.h"
#include "utils.h"

/* libraries */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>
#include <crypt.h>
#include <sys/types.h>
#include <semaphore.h>

/* cracker2.h */
typedef struct _thread_job_t {
    char *name;
    char *hash;
    char *pass;
} thread_job_t;

typedef struct _thread_mailbox_t {
    size_t id;
    char *name;
    char *hash;
    char *pass;
    long count;
    long c;
    long start_index;
    int  state; //0 running, 1 end, 2 found, 3 cancelled
    double time_elapsed;
} thread_mailbox_t;

size_t _thread_count;
int flag = 0;

pthread_mutex_t m;
pthread_mutex_t m2;

pthread_t *thread_pool;
thread_mailbox_t *mailboxes;
thread_job_t *parse_stdin_line();

void thread_routine(void *arg);

/* cracker2.c */
int start(size_t thread_count) {
    _thread_count = thread_count;

    pthread_mutex_init(&m, NULL);
    pthread_mutex_init(&m2, NULL);

    /* create threads & mailboxes */
    mailboxes   = malloc(sizeof(thread_mailbox_t) * thread_count);
    thread_pool = malloc(sizeof(pthread_t)        * thread_count);


    /* run jobs */
    thread_job_t *job;
    while ( (job = parse_stdin_line()) ) {
        flag = 0;

        /* setup each mailbox */
        int password_length = strlen(job->pass);
        int prefix_length   = getPrefixLength(job->pass);
        long start_index;
        long count;

        for (size_t i = 0; i < thread_count; i++) {
            getSubrange(password_length - prefix_length, thread_count, i + 1, &start_index, &count);
            mailboxes[i].id   = i + 1;
            mailboxes[i].name = strdup(job->name);
            mailboxes[i].hash = strdup(job->hash);
            mailboxes[i].pass = strdup(job->pass);
            setStringPosition(mailboxes[i].pass + prefix_length, start_index);
            mailboxes[i].start_index = start_index;
            mailboxes[i].count = count;
            mailboxes[i].c     = 0;
            mailboxes[i].state = 0;
            mailboxes[i].time_elapsed = 0;
        }

        v2_print_start_user(job->name);

        /* clean up job */
        free(job->name); // name, pass, hash are all in the same block
        free(job);

        double start_time = getTime();

        /* start worker threads */
        for (size_t i = 0; i < thread_count; i++) {
            pthread_create(thread_pool + i, NULL, (void *) &thread_routine, (void *) (mailboxes + i));
        }

        /* wait for worker threads to exit */
        for (size_t i = 0; i < thread_count; i++) {
            pthread_join(thread_pool[i], NULL);
        }

        /* read mailboxes */
        int result = 1;
        char *password = NULL;
        int c = 0;
        double cpu_time = 0;

        for (size_t i = 0; i < thread_count; i++) {
            if (mailboxes[i].state == 2) {
                result = 0;
                password = mailboxes[i].pass;
            }
            c += mailboxes[i].c;
            cpu_time += mailboxes[i].time_elapsed;
        }

        pthread_mutex_lock(&m);
        v2_print_summary(mailboxes[0].name, password, c, getTime() - start_time, cpu_time, result);
        pthread_mutex_unlock(&m);

        /* clean up mailbox */
        for (size_t i = 0; i < thread_count; i++) {
            free(mailboxes[i].name);
            free(mailboxes[i].hash);
            free(mailboxes[i].pass);
        }
    }

    // clean up
    free(mailboxes);
    free(thread_pool);
    pthread_mutex_destroy(&m);
    pthread_mutex_destroy(&m2);
    return 0;
}

// parses a single line from stdin for a job
thread_job_t *parse_stdin_line() {
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

        thread_job_t *job = malloc(sizeof(thread_job_t));
        job->name = name;
        job->hash = hash;
        job->pass = pass;
        return job;
    }

    free(line);
    return NULL;
}

void thread_routine(void *arg) {
    thread_mailbox_t *mailbox = (thread_mailbox_t *) arg;
    double start_time = getThreadCPUTime();

    pthread_mutex_lock(&m);
    v2_print_thread_start(mailbox->id, mailbox->name, mailbox->start_index, mailbox->pass);
    pthread_mutex_unlock(&m);

    struct crypt_data cdata;
    cdata.initialized = 0;
    const char *hash;

    for (mailbox->c = 1; mailbox->c <= mailbox->count; mailbox->c++) {
        hash = crypt_r(mailbox->pass, "xx", &cdata);

        if (!strcmp(hash, mailbox->hash)) {
            mailbox->state = 2;

            pthread_mutex_lock(&m2);
            flag = 1;
            pthread_mutex_unlock(&m2);

            pthread_mutex_lock(&m);
            v2_print_thread_result(mailbox->id, mailbox->c, 0);
            pthread_mutex_unlock(&m);
            mailbox->time_elapsed = getThreadCPUTime() - start_time;
            return;
        }
        incrementString(mailbox->pass);

        pthread_mutex_lock(&m2);
        if (flag) {
            pthread_mutex_unlock(&m2);

            mailbox->state = 3;
            pthread_mutex_lock(&m);
            v2_print_thread_result(mailbox->id, mailbox->c, 1);
            pthread_mutex_unlock(&m);
            mailbox->time_elapsed = getThreadCPUTime() - start_time;
            return;
        }
        pthread_mutex_unlock(&m2);
    }

    mailbox->state = 1;

    pthread_mutex_lock(&m);
    v2_print_thread_result(mailbox->id, mailbox->c, 2);
    pthread_mutex_unlock(&m);
    mailbox->time_elapsed = getThreadCPUTime() - start_time;
}
