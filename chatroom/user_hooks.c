/**
 * Chatroom Lab
 * CS 241 - Fall 2016
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define READ_FAIL_NUM 10

static size_t read_counter;

int my_read(int fd, void *buff, size_t count) {
  read_counter++;

  if (read_counter % 4 == 0) {
    read_counter = 0;

    if (rand() % 2 > 0) {
      return read(fd, buff, count / 2 + 1);
    } else {
      errno = EINTR;
      return -1;
    }
  }

  return read(fd, buff, count);
}
