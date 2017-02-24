/**
 * Chatroom Lab
 * CS 241 - Fall 2016
 */
#include "camelCaser_tests.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "camelCaser.h"

/*
 * Testing function for various implementations of camelCaser.
 *
 * @param  camelCaser   A pointer to the target camelCaser function.
 * @return              Correctness of the program (0 for wrong, 1 for correct).
 */

void freeOutput(char **output, int deep) {
	if (deep) {
		char* string;
		int iter = 0;

		while ((string = output[iter++])) {
			free(string);
		}
	}
	free(output);
}

int test_camelCaser(char **(*camelCaser)(const char *)) {
	char* string;
	int iter;

	// // TEST A
	char **solution_a = malloc(10*sizeof(char*));
	solution_a[0] = "";
	solution_a[1] = "1";
	solution_a[2] = "2";
	solution_a[3] = "3";
	solution_a[4] = "4";
	solution_a[5] = "5";
	solution_a[6] = "6app\5\6\20\31leFuck";
	solution_a[7] = "f7ckMe";
	solution_a[8] = "8";
	solution_a[9] = NULL;

    char **a = (*camelCaser)(".1/2;3'4[5]6app\5\6\20\31le fuck,f7ck me(8)9");

	// Test a and solution_a are the same length
	if (sizeof(a)/sizeof(char*) != sizeof(solution_a)/sizeof(char*)) {
		freeOutput(a, 1);
		freeOutput(solution_a, 0);
		return 0;
	} 

	// Then test that they contain the same strings
	iter = 0;
	while ((string = a[iter])) {
		if (strcmp(string, solution_a[iter])) {
			freeOutput(a, 1);
			freeOutput(solution_a, 0);
			return 0;
		}
		iter++;
	}

	freeOutput(a, 1);
	freeOutput(solution_a, 0);

	// TEST B
	char **solution_b = malloc(7*sizeof(char*));
	solution_b[0] = "heisenbugIncredible";
	solution_b[1] = "";
	solution_b[2] = "";
	solution_b[3] = "a";
	solution_b[4] = "fuckmeSilly";
	solution_b[5] = "lulLulLemonadeMakesTheBestFuud";
	solution_b[6] = NULL;

	char **b = (*camelCaser)("Heisenbug incredible.. . a . fuckme SIlly. Lul Lul lemonade makes the best fUUd. ignore this bit Facenovel");

	// // Test b and solution_b are the same length
	if (sizeof(b)/sizeof(char*) != sizeof(solution_b)/sizeof(char*)) {
		freeOutput(b, 1);
		freeOutput(solution_b, 0);
		return 0;
	} 

	// Then test that they contain the same strings
	iter = 0;
	while ((string = b[iter])) {
		if (strcmp(string, solution_b[iter])) {
			freeOutput(b, 1);
			freeOutput(solution_b, 0);
			return 0;
		}
		iter++;
	}

	freeOutput(b, 1);
	freeOutput(solution_b, 0);

	// TEST C
	char **c = (*camelCaser)(NULL);

	if (c != NULL)
		return 0;


    return 1;
}
