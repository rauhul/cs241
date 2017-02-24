/**
 * Machine Problem: Vector
 * CS 241 - Fall 2016
 */

#include "document.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
	Document *document;

	// TEST 1
	document = Document_create();
	Document_insert_line(document, 1, "line1");
	Document_insert_line(document, 2, "line2");
	Document_set_line(document,    1, "newline1");
	Document_insert_line(document, 4, "line4");
	Document_write_to_file(document,  "output");
	Document_destroy(document);


	// TEST 2
	document = Document_create_from_file("output");

	assert(*Document_get_line(document, 1) == 'n');
	assert(*Document_get_line(document, 2) == 'l');
	assert( Document_get_line(document, 3) == NULL);
	assert(*Document_get_line(document, 4) == 'l');


	// TEST 3
	Document_delete_line(document, 3);
	assert(*Document_get_line(document, 1) == 'n');
	assert(*Document_get_line(document, 2) == 'l');
	assert(*Document_get_line(document, 3) == 'l');
	Document_destroy(document);


	// TEST 4
	document = Document_create();
	Document_write_to_file(document,  "output");
	Document_destroy(document);

	document = Document_create_from_file("output");
	Document_destroy(document);

	return 0;
}
