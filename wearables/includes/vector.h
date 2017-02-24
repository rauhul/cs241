/**
 * Machine Problem: Wearables
 * CS 241 - Fall 2016
 */

#pragma once
#include <stdlib.h>

/**
 * An automatically expanding array that can store any kind of homogenous data
 *
 * Note: NULL is considered to be a valid entry. This means users can insert
 * a NULL into the vector if they choose to.
 *
 * As entries are added to it, the array will automatically grow
 * (sort of like a C++ vector or Java ArrayList).
 *
 * 'size' is the number of actual objects held in the vector,
 * which is not necessarily equal to its storage capacity.
 *
 * 'capacity' is the storage space currently allocated for the vector,
 * expressed in terms of elements. This is also the maximum number of elements
 * the array can currently hold.
 * 'size' will always be less than or equal to 'capacity'.
 *
 * The reason size and capacity are two different variables is that
 * the extra space allows us to accommodate for growth without
 * the need to reallocate on each insertion.
 *
 * Understand the difference between size and capacity! The last element of
 * the vector is at index 'size - 1', but the array reserves memory for
 * 'capacity' elements.
 *
 * The vector is initialized with a capacity of 10 and size of 0. The user can
 * add up to 10 entries and the array will not need to be reallocated--
 * just change 'size'. For example, if you have inserted two entries into
 * the vector, the capacity will still be 10, but the size will be 2.
 *
 * When the user wants to add the 11th entry, the array will be reallocated
 * to hold 20 entries. If that fills up, reallocate it to hold 40 entries,
 * and so on.
 *
 * Why double the capacity each time instead of just adding another 10 elements?
 * Reallocations can be expensive, and the performance will probably suffer
 * if you make a lot of them.
 *
 * When the vector is relatively empty (that is, when its size is 1/4th of
 * its capacity), it should be reduced to half capacity to free up space.
 */

/* Forward declare vector structure. */
struct Vector;
typedef struct Vector Vector;

/**
 * This callback function takes in a pointer to arbitary data
 * and returns a void pointer pointing to a copy of said data.
*/
typedef void *(*copy_constructor_type)(void *elem);

/**
 * This callback function takes in a pointer to arbitary data
 * and frees the memory allocated for it.
*/
typedef void (*destructor_type)(void *elem);

/* Allocate and return a new Vector structure. */
Vector *Vector_create(copy_constructor_type c, destructor_type d);

/**
 * Deallocate a Vector structure.
 *
 * Every non-NULL entry in array from array[0] to array[size-1] will
 * need to be 'destroyed'.
*/
void Vector_destroy(Vector *vector);

/* Return the number of entries in use in this array. */
size_t Vector_size(Vector *vector);

/* Return the capacity of the vector. */
size_t Vector_capacity(Vector *vector);

/**
 * Sets the size of the array.
 *
 * If the array grows, new entries will be initialized to NULL.
 * If the array shrinks, entries past the new end of the array
 * will be deallocated.
 *
 * If the 'new_size' is larger than the capacity of the array, the
 * capacity will need to be increased, meaning the array will need to
 * be reallocated. You should keep doubling the capacity until it is bigger
 * than the 'new_size'.
 *
 * Also, if the new size of vector is less than or equal to 1/4th the capacity,
 * then you should keep halving the capacity until
 * halving one more time would make capacity <= new size.
 * However, to avoid repeated realloc()s with small capacity,
 * maintain a minimum capacity of 10.
*/
void Vector_resize(Vector *vector, size_t new_size);

/**
 * Allocate a copy of 'elem' and store it in array[index].
 * If 'elem' is NULL, just store NULL in array[index].
 *
 * Note that if the vector is allocating memory for the entry
 * (which will happen when the entry is not NULL),
 * then it should also be the freeing it (hint, hint).
*/
void Vector_set(Vector *vector, size_t index, void *elem);

/**
 * Returns the entry stored at array[index].
 * If array[index] == NULL, this will return NULL.
 *
 * This does not return a copy of the entry; it returns the actual
 * void* stored at array[index]. This means the caller must not
 * deallocate the entry (if the entry is non-null).
*/
void *Vector_get(Vector *vector, size_t index);

/**
 * Inserts a copy of 'elem' into array[index].
 * If 'elem' is NULL, then NULL is inserted.
 *
 * If the index is past the last element of the vector, it should resize() to
 * accommodate the new entry.
 *
 * Otherwise, the existing entry at array[index] and any subsequent entries
 * should be shifted towards the end of the array, and the vector should be
 * resized accordingly.
*/
void Vector_insert(Vector *vector, size_t index, void *elem);

/**
 * Removes whatever entry is contained in array[index].
 *
 * You should also shift any subsequent entries towards the beginning of
 * the array. Do not forget to resize the vector accordingly.
*/
void Vector_delete(Vector *vector, size_t index);

/**
 * Appends a copy of 'elem' (or NULL) right after the last element in the
 * vector.
*/
void Vector_append(Vector *vector, void *elem);
