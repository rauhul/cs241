/**
 * Chatroom Lab
 * CS 241 - Fall 2016
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>

#include "utils.h"

#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define _LOG 0
#define LOG(format, ...) if (_LOG) { \
    fprintf(stderr, ANSI_COLOR_BLUE format ANSI_COLOR_MAGENTA "\t(%s, %s::%d)\n" ANSI_COLOR_RESET, \
    ## __VA_ARGS__, __FILE__, __func__, __LINE__); }

static const size_t MESSAGE_SIZE_DIGITS = 4;

char *create_message(char *name, char *message) {
    int name_len = strlen(name);
    int msg_len = strlen(message);
    char *msg = calloc(1, msg_len + name_len + 4);
    sprintf(msg, "%s: %s", name, message);

    return msg;
}

ssize_t get_message_size(int socket) {
    int32_t size;
    ssize_t read_bytes =
        read_all_from_socket(socket, (char *)&size, MESSAGE_SIZE_DIGITS);
    if (read_bytes == 0 || read_bytes == -1)
        return read_bytes;

    return (ssize_t)ntohl(size);
}

ssize_t read_all_from_socket(int socket, char *buffer, size_t count) {
    ssize_t bytes_read = 0;
    ssize_t bytes_remaining = count;
    ssize_t retval;

    while (bytes_remaining > 0 || (retval == -1 && errno == EINTR)) {
        retval = read(socket, (void *) (buffer + bytes_read), bytes_remaining);
        if (retval == 0)
            return 0;
        if (retval > 0) {
            bytes_read      += retval;
            bytes_remaining -= retval;
        }

    }
    return count;
}

ssize_t write_message_size(size_t size, int socket) {
    int32_t message_size = htonl(size);

    ssize_t write_bytes =
        write_all_to_socket(socket, (char *)&message_size, MESSAGE_SIZE_DIGITS);

    return write_bytes;
}

ssize_t write_all_to_socket(int socket, const char *buffer, size_t count) {
    ssize_t bytes_read = 0;
    ssize_t bytes_remaining = count;
    ssize_t retval;

    while (bytes_remaining > 0 || (retval == -1 && errno == EINTR)) {
        retval = write(socket, (void *) (buffer + bytes_read), bytes_remaining);
        if (retval == 0)
            return 0;
        if (retval > 0) {
            bytes_read      += retval;
            bytes_remaining -= retval;
        }
    }
    return count;
}