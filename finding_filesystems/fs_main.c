/**
* Finding Filesystems
* CS 241 - Fall 2016
*/
#include "fs.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

void print_general_help() {
  printf("Help: File System Navigator\n\n");
  printf("./minixfs <file_system> ls <path>\n");
  printf("./minixfs <file_system> cat  <path>\n");
  printf("./minixfs <file_system> stat <path>\n");
}

int main(int argc, char *argv[]) {

  if (argc < 2) {
    print_general_help();
    return 1;
  }
  argv++;
  file_system *fs = open_fs(argv[0]);
  if (!fs) {
    fprintf(stderr, "Filesystem not found: %s\n", argv[0]);
    return 1;
  }
  argv++;
  if (!strcmp(*argv, "ls")) {
    char *path = *++argv;
    if (path) {
      fs_ls(fs, path);
    } else {
      print_general_help();
    }
  } else if (!strcmp(*argv, "cat")) {
    fs_cat(fs, *++argv);
  } else if (!strcmp(*argv, "stat")) {
    fs_stat(fs, *++argv);
  } else {
    printf("Invalid command");
    print_general_help();
  }
  close_fs(&fs);

  return 0;
}
