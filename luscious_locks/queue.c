/**
 * Luscious Locks Lab
 * CS 241 - Fall 2016
 */
#include "queue.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * Struct representing a node in a queue_t
 */
typedef struct queue_node_t {
    struct queue_node_t *next;
    void *data;
} queue_node_t;

/**
 * Struct representing a queue
 */
struct queue_t {
    queue_node_t *head, *tail;
    int size;
    int maxSize;
    pthread_cond_t cv;
    pthread_mutex_t m;
};

/**
 *  Given data, place it on the queue.  Can be called by multiple threads.
 *  Blocks if the queue is full.
 */
void queue_push(queue_t *queue, void *data) {
    pthread_mutex_lock(&(queue->m));

    while (queue->size >= queue->maxSize) {
        pthread_cond_wait(&(queue->cv), &(queue->m));
    }

    queue_node_t *node = malloc(sizeof(queue_node_t));
    node->next = NULL;
    node->data = data;

    if (queue->size == 0) {
        queue->head = queue->tail = node;
    } else {
        queue->tail->next = node;
        queue->tail = node;
    }

    queue->size += 1;
    pthread_cond_broadcast(&(queue->cv));
    pthread_mutex_unlock(&(queue->m));
}

/**
 *  Retrieve the data from the front of the queue.  Can be called by multiple
 * threads.
 *  Blocks if the queue is empty.
 */
void *queue_pull(queue_t *queue) {
    pthread_mutex_lock(&(queue->m));

    while (queue->size <= 0) {
        pthread_cond_wait(&(queue->cv), &(queue->m));
    }

    queue_node_t *node;
    node = queue->head;

    if (queue->size == 1) {
        queue->head = queue->tail = NULL;
    } else {
        queue->head = node->next;
    }

    void *data = node->data;
    free(node);

    queue->size -= 1;
    pthread_cond_broadcast(&(queue->cv));
    pthread_mutex_unlock(&(queue->m));

    return data;
}

/**
 *  Allocates heap memory for a queue_t and initializes it.
 *  Returns a pointer to this allocated space.
 */
queue_t *queue_create(int maxSize) {
    queue_t *queue = malloc(sizeof(queue_t));
    if (!queue)
        return NULL;

    queue->head = NULL;
    queue->tail = NULL;

    queue->size = 0;
    queue->maxSize = maxSize;

    pthread_mutex_init(&(queue->m), NULL);
    pthread_cond_init(&(queue->cv), NULL);

    return queue;
}

/**
 *  Destroys the queue, freeing any remaining nodes in it.
 */
void queue_destroy(queue_t *queue) {
    if (!queue)
        return;

    queue_node_t *iter = queue->head;
    queue_node_t *curr;
    while (iter) {
        curr = iter;
        iter = iter->next;
        free(curr);
    }

    pthread_cond_destroy(&(queue->cv));
    pthread_mutex_destroy(&(queue->m));

    free(queue);
}