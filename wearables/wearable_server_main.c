/**
 * Machine Problem: Wearables
 * CS 241 - Fall 2016
 */

#include <stdio.h>
#include <stdlib.h>

#include "wearable_server.h"

int main(int argc, const char *argv[]) {
  if (argc != 3) {
    printf("Usage:\n\t%s wearable_port request_port\n", argv[0]);
    exit(1);
  }

  wearable_server(argv[1], argv[2]);
}
