/**
 * Machine Problem: Shell
 * CS 241 - Fall 2016
 */
#ifndef __CS241_VECTOR_H__
#define __CS241_VECTOR_H__
#include <stdlib.h>

/**
 * An automatically-expanding array of strings.
 *
 * Note: NULL is considered to be a valid entry. This means users can insert a
 * NULL into the vector if they choose to.
 *
 * As entries are added to it, the array will automatically grow.
 * (sort of like a C++ vector<string> or Java ArrayList<String>)
 *
 * 'size' is the number of elements in array[] that may be accessed
 * by Vector_set() and Vector_get(). Calling Vector_set() or Vector_get()
 * with index < 0 or index >= size is an error and should cause
 * an assertion failure.
 *
 * The size of the array can be changed with Vector_resize() or
 * Vector_append().
 *
 * 'capacity' is number of elements currently allocated for array,
 * that is, the maximum number of elements the array can currently hold.
 * 'size' will always be less than or equal to 'capacity'.
 *
 * The reason size and capacity are two different variables is so that
 * we don't have to reallocate every time an entry is added to the
 * array.

 * The vector is initialized with a capacity of 10 and size of 0.  The user can
 * add up to 10 entries and array will not need to be reallocated-- just change
 * 'size'.  For example, if you have inserted two entries into the vector, the
 * capacity will still be 10, but the size will be 2.
 *
 * When the user wants to add the 11th entry, the array will be reallocated to
 * hold 20 entries. If that fills up, reallocate it to hold 40 entries, and so
 * on.

 * Why double the size each time rather than just adding another 10
 * elements? Reallocations can be expensive, and the performance
 * will probably suffer if you do a lot of them.
 *
 */

typedef struct {
  /**
   * An array of pointers to the entries in the array.
   * NULL values are allowed.
  */
  char **array;

  /* The current size of the 'vector' i.e., the number of elements the user
   * knows about*/
  size_t size;

  /* The allocated size of the 'array' i.e., the number of elements the 'array'
   * can _actually_ hold. */
  size_t capacity;
} Vector;

/* Allocate and return a new Vector structure. */
Vector *Vector_create();

/**
 * Deallocate a Vector structure.
 * Every non-NULL entry in array from array[0] to array[size-1] will
 * need to be free()d.
*/
void Vector_destroy(Vector *vector);

/* Return the number of entries in use in this array. */
size_t Vector_size(Vector *vector);

/**
 * Sets the size of the array.
 *
 * If the array grows, new entries will be initialized to NULL.
 * If the array shrinks, entries past the new end of the array
 * will be deallocated.
 *
 * If the new size is larger than the capacity of the array, the
 * capacity will need to be increased, meaning the array will need to
 * be reallocated.  The new capacity should be at least twice as big
 * as it was before. In other words:
 *
 * if new_size > capacity:
 *      new_capacity = maximum(capacity * 2, new_size);
*/
void Vector_resize(Vector *vector, size_t new_size);

/**
 * Allocate a copy of 'str' and store it in array[index].  If 'str' is NULL,
 * just store NULL in array[index]. Note that if the vector is allocating
 * memory for the entry (which will happen when th entry is not NULL), then it
 * should also be the freeing it (hint,hint)
*/
void Vector_set(Vector *vector, size_t index, const char *str);

/**
 * Returns the entry stored at array[index].
 * If array[index]==NULL, this will return NULL.
 *
 * This does not return a copy of the entry; it returns the actual
 * char* stored at array[index]. This means the caller must not
 * deallocate the entry (if the entry is non-null).
*/
const char *Vector_get(Vector *vector, size_t index);

/**
 * Inserts a copy of 'str' or NULL if str is NULL and stores it in array[index].
 *
 * Note the array[index] could already contain a entry, in which case the entry
 * at array[index] and all subsequent entries should be shifted down.
 *
 * Also note that index could be well beyond the position of the last entry. In
 * which case the vector should resize() to barely accommodate that new entry.
*/
void Vector_insert(Vector *vector, size_t index, const char *str);

/**
 * Removes whatever entry is contained in array[index].
 *
 * Note that index does not have to be the last element of the vector, in which
 * case you should not only remove the entry at array[index], but also shift all
 * subsequent entries down.
 *
 * Do not forget to resize the vector accordingly.
*/
void Vector_delete(Vector *vector, size_t index);

/**
 * Appends a copy of 'str' (or NULL) right after the last element in the vector.
*/
void Vector_append(Vector *vector, const char *str);

#endif /* __CS241_VECTOR_H__ */
