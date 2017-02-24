/**
 * Mini Valgrind Lab
 * CS 241 - Fall 2016
 */

#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <assert.h>

#include "mini_valgrind.h"

#define TEST_SIZE_STANDARD 8
#define TEST_SIZE_LARGE    16
#define TEST_SIZE_SMALL    4

int main() {
    void *test;
    
    // TEST 1 - MALLOC/FREE
    test = malloc(TEST_SIZE_STANDARD);
    free(test);


    // TEST 2 - MALLOC/REALLOC SMALLER/FREE
    test = malloc(TEST_SIZE_STANDARD);
    test = realloc(test, TEST_SIZE_SMALL);
    free(test);


    // TEST 3 - MALLOC/REALLOC LARGER/FREE
    test = malloc(TEST_SIZE_STANDARD);
    test = realloc(test, TEST_SIZE_LARGE);
    free(test);


    // TEST 4 - MALLOC/REALLOC SAME SIZE/FREE
    test = malloc(TEST_SIZE_STANDARD);
    test = realloc(test, TEST_SIZE_STANDARD);
    free(test);


    // TEST 5 - BAD FREE
    free(test);


    // TEST 6 - FREE NULL
    test = NULL;
    free(test);


    // TEST 7 - MALLOC/ASSIGN/REALLOC SMALLER/REASSIGN/REALLOC LARGER/FREE
    test = malloc(TEST_SIZE_STANDARD);

    strcpy((char *) test, "1234567");
    assert(*(char *) test == '1');

    test = realloc(test, TEST_SIZE_SMALL);
    assert(*(char *) test == '1');

    strcpy((char *) test, "253");
    assert(*(char *) test == '2');

    test = realloc(test, TEST_SIZE_LARGE);
    assert(*(char *) test == '2');

    free(test);


    // TEST 8 - MALLOCx3/OUT OF ORDER FREEx3
    void *test1 = malloc(TEST_SIZE_STANDARD);
    void *test2 = malloc(TEST_SIZE_STANDARD);
    void *test3 = malloc(TEST_SIZE_STANDARD);

    free(test2);
    free(test1);
    free(test3);


    // REQUIRED
    atexit(print_report);
    return 0;
}
