/**
 * Machine Problem: Text Editor
 * CS 241 - Fall 2016
 */

#ifndef __CS241_FORMAT_H__
#define __CS241_FORMAT_H__
#include <stdio.h>

/**
 * This library will handle all the formating
 * for your document and text editor.
 *
 * Please use this to ensure that your formatting
 * matches what the autograder expects.
*/

/**
 * Call this function when your user has incorrectly
 * executed your text editor.
 */
void print_usage_error();

/**
 * Prints out one line from 'document'.
 *
 * The index parameter is 0-indexed. This function adds 1 to the index when
 * printing (because the editor is 1-indexed) (check format.c).
 */
void print_line(Document *document, size_t index);

/**
 * Error message that should be displayed if the document is empty and the user
 * attempts to display something.
 */
void print_document_empty_error();

/**
 * Error message that should be displayed if the user wants to display,
 * write/append to, or delete an invalid line.
 */
void invalid_line();

/**
 * Error message that should be displayed if the user tries to execute an
 * invalid command.
 */
void invalid_command(const char *command);

/**
 * Prints a line from the Document with the first occurrence of the search term
 * in square brackets given the following:
 *
 * 'line_number' is the line_number of the line.
 * 'line' is the entire line that contains the search term.
 * 'start' is a pointer to the start of the search term.
 * 'search_term' is the term the user is searching for.
 */
void print_search_line(int line_number, const char *line, const char *start,
                       const char *search_term);

/**
 * Splits a copy of 'str' by a 'delim'.
 * Then writes the number of tokens into 'numtokens'.
 * Learn more at:
 * https://www.quora.com/How-do-you-write-a-C-program-to-split-a-string-by-a-delimiter
 */
char **strsplit(const char *str, const char *delim, size_t *numtokens);

#endif /* __CS241_FORMAT_H__ */
