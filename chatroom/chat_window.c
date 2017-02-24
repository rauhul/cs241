/**
 * Chatroom Lab
 * CS 241 - Fall 2016
 */

#include <errno.h>
#include <ncurses.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "utils.h"

static volatile int closeChat;

static WINDOW *output;
static WINDOW *input;
static WINDOW *boundary;

static FILE *output_file;

void close_chat() { closeChat = 1; }

void draw_border(WINDOW *screen) {
  int x, y, i;
  // Macro that sets y and x based on the dimensions of 'screen'
  getmaxyx(screen, y, x);
  for (i = 1; i < (x - 1); i++) {
    mvwprintw(screen, y - 1, i, "-");
  }
}

void create_windows(char *filename) {

  // Check if a filename was passed
  // If so, write to it instead of using the ncurses window
  if (filename) {
    output_file = fopen(filename, "w");
    if (output_file == NULL) {
      perror("create_windows() failed to open output file");
      exit(1);
    }
    return;
  }

  // Only create the curses windows if we're not logging to a file

  int parent_x, parent_y;
  int boundary_size = 2;
  initscr();
  curs_set(1); // Sets cursor visibility

  // Get maximum window dimensions
  getmaxyx(stdscr, parent_y, parent_x);

  // Set up initial windows
  output = newwin(parent_y - boundary_size - 1, parent_x, 0, 0);
  input = newwin(1, parent_x, parent_y - 1, 0);
  boundary = newwin(boundary_size, parent_x, parent_y - boundary_size - 1, 0);
  // Allow the windows to scroll
  scrollok(output, TRUE);
  scrollok(input, TRUE);

  draw_border(boundary);
  wrefresh(boundary);
}

void write_message_to_screen(const char *format, ...) {

  va_list arg;
  va_start(arg, format);
  if (output_file) {
    vfprintf(output_file, format, arg);
  } else {
    vwprintw(output, format, arg);
    wrefresh(output);
  }
  va_end(arg);
}

void read_message_from_screen(char **buffer) {
  // Allocate buffer if needed
  if (*buffer == NULL)
    *buffer = calloc(1, MSG_SIZE);
  else
    memset(*buffer, 0, MSG_SIZE);
  int error = OK;
  // Don't print a prompt if we're logging to output file
  if (output_file) {
    if (fgets(*buffer, MSG_SIZE, stdin) == NULL) {
      error = ERR;
    }

    char *ptr = *buffer;
    while (*ptr) {
      if (*ptr == '\n') {
        *ptr = '\0';
        break;
      }
      ptr++;
    }
  } else {
    // Else proceed to curses and print to screen
    // Print a fresh prompt
    wprintw(input, "> ");
    wrefresh(input);
    // Read input on the input window
    error = wgetnstr(input, *buffer, MSG_SIZE - 1);
  }
  if (closeChat || error == ERR) {
    free(*buffer);
    *buffer = NULL;
  }
}

void destroy_windows() {
  // clean up
  delwin(output);
  delwin(boundary);
  delwin(input);
  endwin();

  if (output_file) {
    fclose(output_file);
    output_file = NULL;
  }
}
