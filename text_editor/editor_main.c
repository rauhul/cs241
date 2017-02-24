/**
 * Machine Problem: Text Editor
 * CS 241 - Fall 2016
 */

#include "document.h"
#include "editor.h"
#include "format.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// DFA INPUT CHECKER

int is_valid_command(char *command) {
  int ret = false; // invalid
  char *curr = command;
  int state = 0; // start state
  while (*curr) {
    char c = *curr;
    switch (state) {
    case 0:
      if (c == 'p')
        state = 1;
      else if (c == 'd')
        state = 4;
      else if (c == 'w' || c == 'a')
        state = 5;
      else if (c == '/')
        state = 9;
      else if (c == 's' || c == 'q')
        state = 10;
      else
        state = -1;
      break;
    case 1:
      if (isspace(c))
        state = 2;
      else
        state = -1;
      break;
    case 2:
      if (isdigit(c) && c != '0')
        state = 3;
      else
        state = -1;
      break;
    case 3:
      if (!isdigit(c))
        state = -1;
      break;
    case 4:
      if (isspace(c))
        state = 2;
      else
        state = -1;
      break;
    case 5:
      if (isspace(c))
        state = 6;
      else
        state = -1;
      break;
    case 6:
      if (isdigit(c) && c != '0')
        state = 7;
      else
        state = -1;
      break;
    case 7:
      if (isdigit(c))
        state = 7;
      else if (isspace(c))
        state = 8;
      else
        state = -1;
      break;
    case 8:
      if (!(isprint(c) || isspace(c)))
        state = -1;
      break;
    case 9:
      if (!(isprint(c) || isspace(c)))
        state = -1;
      break;
    case 10:
      if (*curr)
        state = -1;
    default:
      return false;
    }
    curr++;
  }

  if (state == 1 || state == 3 || state == 8 || state == 9 || state == 10) {
    ret = true;
  }

  return ret;
}

/**
 * This is the will be the entry point to your text editor.
*/
int main(int argc, char *argv[]) {
  // Checking to see if the editor is being used correctly.
  if (argc != 2) {
    print_usage_error();
    return 1;
  }
  // Setting up a document based on the file named 'filename'.
  char *filename = get_filename(argc, argv);
  Document *document = Document_create_from_file(filename);

  if (document == NULL) {
    fprintf(stderr, "Failed to create document!\n");
    return 1;
  }

  // Buffer for the command and length of said buffer
  char *command = NULL;
  size_t len = 0;

  // This while loop will keep reading from stdin one line at a time
  // until the user enters 'q' (the quit command).
  int done = 0;
  while (!done) {
    getline(&command, &len, stdin);

    // remove newline from the command
    char *nl = strchr(command, '\n');
    if (nl)
      *nl = 0;

    if (!is_valid_command(command)) {
      invalid_command(command);
      continue;
    }

    int command_type = command[0];
    switch (command_type) {
    case 'p':
      handle_display_command(document, command);
      break;
    case 'w':
      handle_write_command(document, command);
      break;
    case 'a':
      handle_append_command(document, command);
      break;
    case 'd':
      handle_delete_command(document, command);
      break;
    case '/':
      handle_search_command(document, command);
      break;
    case 's':
      if (strlen(command) == 1) {
        handle_save_command(document, filename);
      } else {
        invalid_command(command);
      }
      break;
    case 'q':
      done = 1;
      Document_destroy(document);
      break;
    default:
      fprintf(stderr, "Something bad happened! Report to CS241 Staff!\n");
      break;
    }
  }

  // Need to free the buffer that we created.
  if (command) {
    free(command);
  }
}
