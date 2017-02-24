/**
 * Machine Problem 0
 * CS 241 - Spring 2016
 */

#include "part2-functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * (Edit this function to print out the "Illinois" lines in
 * part2-functions.c in order.)
 */
int main() {
    // 1
    first_step(81);

    // 2
    int a = 132;
    second_step(&a);

    // 3
    a = 8942;
    int *b = &a;
    double_step(&b);

    // 4
    char *c = malloc(5 + sizeof(int));
    * (int *)(c + 5) = 15;
    strange_step(c);
    free(c); c = NULL;

    // 5
    char *d = malloc(4*sizeof(char));
    d[3] = 0;
    empty_step((void *)d);

    // 6
    d[3] = 'u';
    two_step((void *)d, d);
    free(d); d = NULL;

    // 7
    char *e = malloc(5*sizeof(char));
    three_step(e, e+2, e+4);
    free(e); e = NULL;

    // 8
    char *f = malloc(4*sizeof(char));
    f[1] = 0;
    f[2] = 8;
    f[3] = 16;
    step_step_step(f,f,f);
    free(f); f = NULL;

    // 9
    char g = 1; 
    it_may_be_odd(&g, (int) g);

    // 10
    char h[] = " ,CS241";
    tok_step(h);

    // 11
    int *i = malloc(sizeof(int));
    *i = 0x201;
    the_end((void *) i, (void *) i);
    free(i); i = NULL;

    return 0;
}