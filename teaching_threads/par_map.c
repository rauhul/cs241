/**
 * Parallel Map Lab
 * CS 241 - Fall 2016
 */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "map.h"
#include "mappers.h"

/* You should create a struct that will get passed in by reference to your
 * start_routine. */
typedef struct _thread_data {
	mapper map_func;
	size_t start_index;
	size_t end_index;

    double *inout_array;
} thread_data;

/* You should create a start routine for your threads. */
void par_map_routine(void *data) {
	thread_data *t_data = (thread_data *) data;
	for (size_t iter = t_data->start_index; iter < t_data->end_index; iter++) {
		t_data->inout_array[iter] = t_data->map_func(t_data->inout_array[iter]);
	}
}

double *par_map(double *list, size_t list_len, mapper map_func, size_t num_threads) {
	if (!list)
		return NULL;

	double *ret_list = (double *)malloc(sizeof(double) * list_len);
	memcpy(ret_list, list, sizeof(double) * list_len);

	if (!ret_list)
		return NULL;

	size_t num_elements_per_thread = list_len/num_threads;
	if (num_elements_per_thread == 0)
		num_elements_per_thread = 1;

	size_t start_index = 0;
	size_t end_index   = 0;

	thread_data *data  = malloc(sizeof(thread_data)*num_threads);
	pthread_t *threads = malloc(sizeof(pthread_t)  *num_threads);

	for (size_t iter = 0; iter < num_threads; iter++) {

		end_index = iter == num_threads - 1 ? list_len : start_index + num_elements_per_thread;

		data[iter].map_func     = map_func;
		data[iter].start_index  = start_index;
		data[iter].end_index  	= end_index;
		data[iter].inout_array  = ret_list;

		pthread_create ((threads + iter), NULL, (void *) &par_map_routine, (void *) (data + iter));
		start_index = end_index;
	}

	for (size_t iter = 0; iter < num_threads; iter++) {
	    pthread_join(threads[iter], NULL);
	}

	free(data);
	free(threads);
	return ret_list;
}
