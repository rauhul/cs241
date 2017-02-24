/**
 * Chatroom Lab
 * CS 241 - Fall 2016
 */
#include <stdio.h>
#include <stdlib.h>

#include "camelCaser.h"
#include "camelCaser_tests.h"

int main() {
	if (test_camelCaser(&camel_caser)) {
		printf("SUCCESS\n");
	} else {
		printf("FAILED\n");
	}
    return 0;
}

