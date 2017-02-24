/**
 * Parallel Map Lab
 * CS 241 - Fall 2016
 */
#include "map.h"
#include <stdlib.h>

double *map(double *list, size_t length, mapper map_func) {
    double *ret_list = (double *)malloc(sizeof(double) * length);

    for (size_t i = 0; i < length; ++i) {
        ret_list[i] = map_func(list[i]);
    }
    return ret_list;
}
