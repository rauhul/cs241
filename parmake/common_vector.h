/**
 * Parallel Make
 * CS 241 - Fall 2016
 */

#pragma once

/*
 * Copy constructor for pointer when using a vector
 * - This doesn't actually copy anything because it is using
 *   it as a adjacency list. All memory manage should happen
 *   outside the vector.
 */
void *copy_pointer(void *rule_t);

/*
 * Destroy constructor for pointers when using a vector
 * - This doesn't actually destroy anything because it is using
 *   it as a adjacency list. All memory manage should happen
 *   outside the vector.
 */
void destroy_pointer();

/*
 * Copy Constructor for strings
 */
void *copy_string(void *arg);

/*
 * Destroy Constructor for strings
 */
void destroy_string(void *arg);
