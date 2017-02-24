/**
 * Chatroom Lab
 * CS 241 - Fall 2016
 */
#pragma once

void timeitout();
/**
 * Sets up ncurses windows
 * Two windows will be created, one for sending the messages
 * and one for recieving messages.
 * A third 'window' will also be created, but this one only
 * exists as a boundary between the two message windows
 * The created screen will look like this:
 *  ___________________________
 *  |text written here        |
 *  |more text                |
 *  |                         |
 *  |                         |
 *  |                         |
 *  |                         |
 *  |-------------------------|
 *  |> your typing here       |
 *  |_________________________|
 *
 *
 * When filename is not NULL, ncurses window will not be
 * used and printing will be to the file with <filename> instead
 */
void create_windows(char *filename);

/**
 * Writes text to the upper window.
 * Behaves just like printf
 * example:
 *      printf("Name: %s ID: %d\n", name, id);
 *      write_message_to_screen("Name: %s ID: %d\n", name, id);
 *
 * This funciton should be used instead of printf
 */
void write_message_to_screen(const char *format, ...);

/**
 * Reads input from bottom window
 * Requires a double pointer to where you want the text to be stored,
 * similar to getline.
 * This function will allocate memory for *buffer if *buffer is NULL
 * example:
 *      char* buffer = NULL
 *      read_message_from_screen(&buffer);
 *
 * This funciton should be used instead of anyother function you
 * might want to use to read from stdin
 */
void read_message_from_screen(char **buffer);

/**
 * Destroys and deallocates all memory used
 * to create the ncurses windows.
 * This also causes the termianl to go back to normal
 */
void destroy_windows();

/**
 * Sets variable in file to signal no more user
 * input should be accepted.
 */
void close_chat();
