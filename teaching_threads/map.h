/**
 * Parallel Map Lab
 * CS 241 - Fall 2016
 */
#ifndef __CS241_MAP_H__
#define __CS241_MAP_H__

#include "mappers.h"
#include <stdlib.h>

/**
 * Single threaded solution to map().
 *
 * This method takes in a `list` of doubles and returns a list of doubles where
 * every element has had `map_func` applied to it.
 *
 * Note: that this function DOES NOT modify the original list.
 *
 * `list`- is a pointer to the begining of an array of doubles.
 * `length` - is how many doubles are in the array of doubles.
 * `map_func` - is the mapper used to transform a double to another double and
 * is applied to every element of the list.
 */
double *map(double *list, size_t length, mapper map_func);
#endif /* __CS241_MAP_H__ */
