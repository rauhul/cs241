/**
 * Machine Problem: Text Editor
 * CS 241 - Fall 2016
 */

#ifndef EDITOR_H
#define EDITOR_H
#include "document.h"

/**
 * This function gets the filename from argc & argv (it's a one liner).
 * @return NULL if there is no filename
*/
char *get_filename(int argc, char *argv[]);

/**
 * Prints out the content of the document
 * based on the command.
*/
void handle_display_command(Document *document, const char *command);

/**
 * Handles writing to the document
 * based on the command passed in.
*/
void handle_write_command(Document *document, const char *command);

/**
 * Handles appending to the document
 * based on the command passed in.
*/
void handle_append_command(Document *document, const char *command);

/**
 * Handles deleting from the document
 * based on the command passed in.
*/
void handle_delete_command(Document *document, const char *command);

/**
 * Handles searching for information from the document
 * based on the command passed in.
*/
void handle_search_command(Document *document, const char *command);

/**
 * Has the document write itself to a file
 * with the specified filename
*/
void handle_save_command(Document *document, const char *filename);

#endif // #ifndef EDITOR_H
