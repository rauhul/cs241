/**
 * Parallel Make
 * CS 241 - Fall 2016
 */

#pragma once
/**
 * This callback function takes in a pointer to arbitary data
 * and returns a void pointer pointing to a copy of said data.
*/
typedef void *(*copy_constructor_type)(void *elem);

/**
 * This callback function takes in a pointer to arbitary data
 * and destroys the data's memory.
*/
typedef void (*destructor_type)(void *elem);

/**
 * Struct representing a queue
 */
typedef struct queue_t queue_t;

/**
 *  Allocates heap memory for a queue_t and initializes it.
 *  Returns a pointer to this allocated space.
 */
queue_t *queue_create(int maxSize, copy_constructor_type c, destructor_type d);

/**
 *  Destroys the queue, freeing any remaining nodes in it.
 */
void queue_destroy(queue_t *queue);

/**
 *  Given data, place it on the queue.  Can be called by multiple threads.
 *  Blocks if the queue is full.
 */
void queue_push(queue_t *queue, void *data);

/**
 *  Retrieve the data from the front of the queue.  Can be called by multiple
 * threads.
 *  Blocks if the queue is empty.
 */
void *queue_pull(queue_t *queue);
