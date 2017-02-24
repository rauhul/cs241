/**
 * Scheduler Lab
 * CS 241 - Fall 2016
 */

/* libraries */
#include <assert.h>
/* files */
#include "libpriqueue.h"
#include "libscheduler.h"

/* type definitions */
typedef struct _job_t job_t;
typedef struct _core_t core_t;

/**
    Stores information making up a job to be scheduled including any statistics.

    You may need to define some global variables or a struct to store your job
    queue elements.
*/
struct _job_t {
    int job_number;
    int priority;
    int running_time;

    int start_time;
    int arrival_time;
    int end_time;
    int rr_time;

    // int waiting_time;
    // int turnaround_time;
    // int response_time;
};

struct _core_t {
    int free;   // boolean representing whether the core is in use or not
    job_t *job; // the job that the core is currently running, if any
};

/* globals */
priqueue_t pqueue;
int _num_cores;
int _free_cores;
core_t *cores;
scheme_t scheme;

int _num_jobs;
int _waiting_time;
int _turnaround_time;
int _response_time;

/* comparers */
int break_tie(const void *a, const void *b) {
    return comparer_fcfs(a, b);
}

int comparer_fcfs(const void *a, const void *b) {
    int priority_diff = ( ((job_t *) a)->arrival_time - ((job_t *) b)->arrival_time );
    if (priority_diff == 0)
        return 0;
    if (priority_diff > 0)
        return 1;
    else
        return -1;
}

int comparer_ppri(const void *a, const void *b) {
    return comparer_pri(a, b);
}

int comparer_pri(const void *a, const void *b) {
    int priority_diff = ( ((job_t *) a)->priority - ((job_t *) b)->priority );
    if (priority_diff == 0)
        return break_tie(a, b);
    if (priority_diff > 0)
        return 1;
    else
        return -1;
}

int comparer_psjf(const void *a, const void *b) {
    return comparer_sjf(a, b);
}

int comparer_rr(const void *a, const void *b) {
    int priority_diff = ( ((job_t *) a)->rr_time - ((job_t *) b)->rr_time );
    if (priority_diff == 0)
        return 0;
    if (priority_diff > 0)
        return 1;
    else
        return -1;
}

int comparer_sjf(const void *a, const void *b) {
    int priority_diff = ( ((job_t *) a)->running_time - ((job_t *) b)->running_time );
    if (priority_diff == 0)
        return break_tie(a, b);
    if (priority_diff > 0)
        return 1;
    else
        return -1;
}

/* init */
void scheduler_start_up(int numcores, scheme_t s) {
    scheme = s;
    _num_cores = numcores;
    _free_cores = numcores;
    cores = malloc(sizeof(core_t) * numcores);

    _num_jobs        = 0;
    _waiting_time    = 0;
    _turnaround_time = 0;
    _response_time   = 0;

    for (int i = 0; i < numcores; i++) {
        cores[i].free = 1;
        cores[i].job  = NULL;
    }

    switch (s) {
    case FCFS:
        priqueue_init(&pqueue, comparer_fcfs);
        break;

    case PRI:
        priqueue_init(&pqueue, comparer_pri);
        break;

    case PPRI:
        priqueue_init(&pqueue, comparer_ppri);
        break;

    case PSJF:
        priqueue_init(&pqueue, comparer_psjf);
        break;

    case RR:
        priqueue_init(&pqueue, comparer_rr);
        break;

    case SJF:
        priqueue_init(&pqueue, comparer_sjf);
        break;

    default:
        printf("Did not recognize scheme\n");
        exit(1);
    }
}

/* job management */
int scheduler_new_job(int job_number, int time, int running_time, int priority) {
    job_t *job = malloc(sizeof(job_t));

    job->job_number     = job_number;
    job->priority       = priority;
    job->running_time   = running_time;
    job->start_time     = 0;
    job->arrival_time   = time;
    job->end_time       = 0;
    job->rr_time        = 0;
    // job->waiting_time   = 0;
    // job->turnaround_time = 0;
    // job->response_time  = 0;
    _num_jobs += 1;

    if (_free_cores > 0 ) {
        for (int i = 0; i < _num_cores; i++) {
            if (cores[i].free) {
                cores[i].free = 0;
                _free_cores -= 1;
                cores[i].job = job;
                job->start_time = time;
                return i;
            }
        }
        assert(0);
    } else if (scheme == PPRI) {
        job_t *worst_job = NULL;
        int core_id;
        for (int i = 0; i < _num_cores; i++) {
            if (worst_job == NULL ||
                comparer_ppri(cores[i].job, worst_job) == 1) {
                worst_job = cores[i].job;
                core_id = i;
            }
        }

        if (comparer_ppri(worst_job, job) == 1) {
            priqueue_offer(&pqueue, worst_job);
            cores[core_id].job = job;
            job->start_time = time;
            job->rr_time = time;
            return core_id;
        }
    } else if (scheme == PSJF) {
        job_t *worst_job = NULL;
        int core_id;
        for (int i = 0; i < _num_cores; i++) {
            if (worst_job == NULL ||
                comparer_psjf(cores[i].job, worst_job) == 1) {
                worst_job = cores[i].job;
                core_id = i;
            }
        }

        if (comparer_psjf(worst_job, job) == 1) {
            priqueue_offer(&pqueue, worst_job);
            cores[core_id].job = job;
            job->start_time = time;
            job->rr_time = time;
            return core_id;
        }
    }
    job->rr_time = time;
    priqueue_offer(&pqueue, job);

    return -1;
}

int scheduler_job_finished(int core_id, int job_number, int time) {
    job_t *job = cores[core_id].job;
    job->end_time = time;

    _waiting_time    += job->start_time - job->arrival_time;
    _turnaround_time += job->end_time   - job->arrival_time;
    _response_time   += job->end_time   - job->start_time;
    free(job);

    if (priqueue_size(&pqueue) > 0) {
        job = (job_t *) priqueue_poll(&pqueue);
        cores[core_id].job = job;
        if (job->start_time == 0 && job->job_number != 0) {
            job->start_time = time;
        }
        job->rr_time = time;
        return job->job_number;
    }

    cores[core_id].job = NULL;
    cores[core_id].free = 1;
    _free_cores += 1;
    return -1;
}

int scheduler_quantum_expired(int core_id, int time) {
    job_t *job = cores[core_id].job;

    if (job != NULL) {
        job->rr_time = time;
        priqueue_offer(&pqueue, (void *) job);
        cores[core_id].job = NULL;
        cores[core_id].free = 1;
        _free_cores += 1;
    }

    if (cores[core_id].free && priqueue_size(&pqueue) > 0) {
        cores[core_id].free = 0;
        _free_cores -= 1;
        job = (job_t *) priqueue_poll(&pqueue);
        if (job->start_time == 0 && job->job_number != 0) {
            job->start_time = time;
        }
        cores[core_id].job = job;
        job->rr_time = time;
        return job->job_number;
    }
    return -1;
}

/* scheduler info */
float scheduler_average_waiting_time() {
    return _waiting_time/((float) _num_jobs);
}

float scheduler_average_turnaround_time() {
    return _turnaround_time/((float) _num_jobs);
}

float scheduler_average_response_time() {
    return _response_time/((float) _num_jobs);
}

/* memory management */
void scheduler_clean_up() {
    for (int i = 0; i < _num_cores; i++) {
        free(cores[i].job);
    }
    free(cores);
    while(priqueue_size(&pqueue)) {
        free(priqueue_poll(&pqueue));
    }
    priqueue_destroy(&pqueue);
}

/* debugging */
void scheduler_show_queue() {
    printf("size - %d\n", priqueue_size(&pqueue));
}