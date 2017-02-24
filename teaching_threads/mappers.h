/**
 * Parallel Map Lab
 * CS 241 - Fall 2016
 */
#ifndef __CS241_MAPPERS_H__
#define __CS241_MAPPERS_H__
/**
 * This callback function takes in a double and returns a double.
 */
typedef double (*mapper)(double elem);

/**
 * Returns the mapper that matches the name 'mapper_name'
 */
mapper get_mapper(char *mapper_name);

// Callback functions
double triple(double elem);
double negate(double elem);
double slow(double elem);
#endif /* __CS241_MAPPERS_H__ */
