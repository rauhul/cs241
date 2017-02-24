/**
 * Machine Problem: Vector
 * CS 241 - Fall 2016
 */

/* An automatically-expanding array of strings. */
#include "vector.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

#define INITIAL_CAPACITY 10

/*
 * Vector structure
 * Do not modify the structure
 * array: Void pointer to the beginning of an array of void pointers to
 * arbitrary data.
 * size: The number of elements in the vector. This is the number of actual
 * objects held in the vector, which is not necessarily equal to its capacity.
 * capacity: The size of the storage space currently allocated for the vector,
 * expressed in terms of elements.
 * copyt_constructor: the function callback for you to define the way you want
 * to copy elements
 * destructor:  the function callback for you to define the way you want to
 * destroy elements
 */
struct Vector {
    copy_constructor_type copy_constructor;
    destructor_type destructor;

    void **array;
    size_t size;
    size_t capacity;
};

// HELPER FUNCTION - UNSAFE, CALLER MUST PERFORM ALL CHECKS
void Vector_dealloc_elem(Vector *vector, size_t index) {
    void *elem;
    if ((elem = vector->array[index]))
        (*vector->destructor)(elem);
}

// COMPLETE
Vector *Vector_create(copy_constructor_type copy_constructor,
                                            destructor_type destructor) {
    Vector *vector           = malloc(sizeof(Vector));
    if (!vector)
        return NULL;

    vector->copy_constructor = copy_constructor;
    vector->destructor       = destructor;
    vector->array            = malloc(INITIAL_CAPACITY * sizeof(void*));
    for (size_t iter = 0; iter < INITIAL_CAPACITY; iter++)
        vector->array[iter] = NULL;

    vector->size             = 0;
    vector->capacity         = INITIAL_CAPACITY;

    return vector;
}

// COMEPLETE
void Vector_destroy(Vector *vector) {
    assert(vector);

    for (size_t iter = 0; iter < vector->size; iter++)
        Vector_dealloc_elem(vector, iter);

    free(vector->array);
    free(vector);
}

// COMPLETE
size_t Vector_size(Vector *vector) {
    assert(vector);
    return vector->size;
}

// COMPLETE
size_t Vector_capacity(Vector *vector) {
    assert(vector);
    return vector->capacity;
}

// COMPLETE
void Vector_resize(Vector *vector, size_t new_size) {
    assert(vector);
    assert(vector->size != new_size);

    size_t new_capacity = vector->capacity;

    if (new_size > vector->capacity) {
        //.. INCREASE CAPACITY & SIZE

        while (new_capacity < new_size)
            new_capacity *= 2;

        vector->array = realloc(vector->array, new_capacity * sizeof(void*));
        assert(vector->array);
        
        // new entries will be initialized to NULL.
        for (size_t iter = vector->size; iter < new_capacity; iter++)
            vector->array[iter] = NULL;

    } else if (vector->capacity > INITIAL_CAPACITY && 4 * new_size <= vector->capacity) {
        //.. DECREASE CAPACITY & SIZE

        while (new_capacity > 2 * new_size)
            new_capacity /= 2;

        if (new_capacity < INITIAL_CAPACITY)
            new_capacity = INITIAL_CAPACITY;

        // entries past the new end of the array will be deallocated
        for (size_t iter = new_size; iter < vector->capacity; iter++)
            Vector_dealloc_elem(vector, iter);

        vector->array = realloc(vector->array, new_capacity * sizeof(void*));
        assert(vector->array);
    } else {
        // vector capacity ok
        if (new_size > vector->size) {
            //.. INCREASE SIZE
            // new entries will be initialized to NULL.
            for (size_t iter = vector->size; iter < new_size; iter++)
                vector->array[iter] = NULL;

        } else {
            //.. DECREASE SIZE
            // entries past the new end of the array will be deallocated
            for (size_t iter = new_size; iter < vector->size; iter++)
                Vector_dealloc_elem(vector, iter);
        }
    }

    vector->size     = new_size;
    vector->capacity = new_capacity;
}


// COMPLETE
void Vector_set(Vector *vector, size_t index, void *elem) {
    assert(vector);
    assert(index < vector->size);

    void *old;
    if ((old = vector->array[index]))
        (*vector->destructor)(old);

    vector->array[index] = elem ? (*vector->copy_constructor)(elem) : NULL;
}

// COMPLETE
void *Vector_get(Vector *vector, size_t index) {
    assert(vector);
    assert(index < vector->size);
    return vector->array[index];
}

// COMPLETE
void Vector_insert(Vector *vector, size_t index, void *elem) {
    assert(vector);

    if (index < vector->size) {
        Vector_resize(vector, vector->size + 1);
        for (size_t iter = vector->size - 1; iter > index ; iter--)
            vector->array[iter] = vector->array[iter-1];

        vector->array[index] = elem ? (*vector->copy_constructor)(elem) : NULL;

    } else {
        Vector_resize(vector, index + 1);

        if (elem)
            vector->array[index] = (*vector->copy_constructor)(elem);
    }
}

// COMPLETE
void Vector_delete(Vector *vector, size_t index) {
    assert(vector);
    assert(index < vector->size);
    
    // dealloc elem if needed
    Vector_dealloc_elem(vector, index);

    // shift down
    size_t iter;
    for (iter = index; iter < vector->size - 1; iter++) 
        vector->array[iter] = vector->array[iter+1];
    
    vector->array[vector->size - 1] = NULL;

    // reduce size
    Vector_resize(vector, vector->size - 1);
}

// COMPLETE
void Vector_append(Vector *vector, void *elem) {
    assert(vector);

    Vector_resize(vector, vector->size + 1);
    if (elem)
        vector->array[vector->size - 1] = (*vector->copy_constructor)(elem);
}