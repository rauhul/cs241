/**
 * Mini Valgrind Lab
 * CS 241 - Fall 2016
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mini_valgrind.h"
#include "print.h"

#undef malloc
#undef realloc
#undef free

/*
 * Replace normal malloc, this malloc will also create meta data info
 * and insert it to the head of the list.
 * You have to malloc enough size to hold both the size of your allocated
 * and the meta_data structure.
 * In this function, you should only call malloc only once.
 *
 * @param size
 *  Size of the memory block, in bytes.
 * @param file
 *  Name of the file that uses mini_valgrind to detect memory leak.
 * @param line
 *  Line in the file that causes memory leak
 *
 * @return
 *  On success, return a pointer to the memory block allocated by
 *  the function. Note that this pointer should return the actual
 *  address the pointer starts and not the meta_data.
 *
 *  If the function fails to allocate the requested block of memory,
 *  return a null pointer.
 */
void *mini_malloc(size_t size, const char *file, size_t line) {
    // CREATE NODE
    void *memory = malloc(sizeof(meta_data) + size);
    if (!memory)
        return NULL;

    // SPILT META DATA AND REQUESTED MEMORY
    meta_data *meta = (meta_data *) memory;
    void *request   = memory + sizeof(meta_data);

    // ADD META DATA TO LINKED LIST
    insert_meta_data(meta, size, file, line);

    // INCREMENT TOTAL USAGE
    total_usage += size;

    return request;
}

/*
 * Replace normal realloc, this realloc will also first check whether the
 * pointer that passed in has memory. If it has memory then resize the memory
 * to it. Or if the pointer doesn't have any memory, then call malloc to
 * provide memory.
 * For total usage calculation, if the new size is larger than the old size,
 * the total usage should increase the difference between the old size and the
 * new size. If the new size is smeller or equal to the old size, the total
 * usage should remain the same.
 * You have to realloc enough size to hold both the size of your allocated
 * and the meta_data structure.
 * In this function, you should only call malloc only once.
 *
 * @param ptr
 *      The pointer require realloc
 * @param size
 *  Size of the memory block, in bytes.
 * @param file
 *  Name of the file that uses mini_valgrind to detect memory leak.
 * @param line
 *  Line in the file that causes memory leak
 *
 * @return
 *  On success, return a pointer to the memory block allocated by
 *  the function. Note that this pointer should return the actual
 *  address the pointer starts and not the meta_data.
 *
 *  If the function fails to allocate the requested block of memory,
 *  return a null pointer.
 */
void *mini_realloc(void *ptr, size_t size, const char *file, size_t line) {
    if (!ptr)
        return mini_malloc(size, file, line);

    meta_data *prev = NULL;
    meta_data *iter = head;
    meta_data *next;

    while (iter) {
        next = iter->next;
        void *memory = ( (void *) iter ) + sizeof(meta_data);

        // CHECK IF REQUESTED BLOCK
        if (ptr == memory) {

            // UPDATE FILE/LINE IF NO RESIZE IS REQUIRED
            if (size == iter->size) {
                iter->line_num  = line;
                strncpy(iter->file_name, file, MAX_FILENAME_LENGTH - 1); 
                return ptr;
            }

            // REMOVE BLOCK FROM LINKED LIST
            if (prev) {
                prev->next = next;
            } else {
                head = next;
            }

            // INCREMENT TOTAL USAGE
            if (size > iter->size) {
                total_usage += size - iter->size;
            } else if (size < iter->size) {
                total_free  += iter->size - size;
            }

            // REALLOC MEMORY
            void *memory = realloc(iter, sizeof(meta_data) + size);
            if (!memory)
                return NULL;

            // SPILT META DATA AND REQUESTED MEMORY
            meta_data *meta = (meta_data *) memory;
            void *request   = memory + sizeof(meta_data);

            // ADD META DATA TO LINKED LIST
            insert_meta_data(meta, size, file, line);

            return request;
        }
        
        prev = iter;
        iter = next;
    }

    // BLOCK NOT FOUND
    return NULL;
}

/*
 * Replace normal free, this free will also delete the node in the list.
 *
 * @param ptr
 *  Pointer to a memory block previously allocated. If a null pointer is
 *  passed, no action occurs.
 */
void mini_free(void *ptr) {
    if (!ptr)
        return;

    remove_meta_data(ptr);
}

/*
 * Helper function to insert the malloc ptr node to the list.
 * Accumulate total_usage here.
 *
 * @param md
 *  Pointer to the meta_data
 * @param size
 *  Size of the memory block, in bytes.
 * @param file
 *  Name of the file that uses mini_valgrind to detect memory leak.
 * @param line
 *  Line in the file that causes memory leak
 */
void insert_meta_data(meta_data *md, size_t size, const char *file, size_t line) {
    // ASSIGN META DATA
    md->size      = size;
    md->line_num  = line;
    strncpy(md->file_name, file, MAX_FILENAME_LENGTH - 1); 

    // ADD TO LINKED LIST
    md->next      = head;
    head          = md;
}

/*
 * Helper function to remove the free ptr node from the list.
 * Add to total_free here.
 *
 * @param ptr
 *  Pointer to a memory block previously allocated.
 */
void remove_meta_data(void *ptr) {
    meta_data *prev = NULL;
    meta_data *iter = head;
    meta_data *next;

    while (iter) {
        next = iter->next;
        void *memory = ( (void *) iter ) + sizeof(meta_data);

        // CHECK IF REQUESTED BLOCK
        if (ptr == memory) {

            // REMOVE BLOCK FROM LINKED LIST
            if (prev) {
                prev->next = next;
            } else {
                head = next;
            }

            // INCREMENT TOTAL FREE
            total_free += iter->size;

            // DEALLOC BLOCK
            free(iter);

            // EXIT FUNCTION
            return;
        }
        
        prev = iter;
        iter = next;
    }

    // INCREASE BAD_FREES IF THE BLOCK IS NOT FOUND
    bad_frees++;
}

/*
 * Deletes all nodes from the list. Called when the program exits so make sure
 * to not count these blocks as freed.
 */
void destroy() {
    meta_data *iter = head;
    meta_data *next = NULL;
    while (iter) {
        next = iter->next;
        free(iter);
        iter = next;
    }
}

/*
 * Print mini_valgrind leak report.
 */
void print_report() {
    print_leak_info(head, total_usage, total_free, bad_frees);
}