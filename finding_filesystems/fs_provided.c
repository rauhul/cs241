/**
* Finding Filesystems
* CS 241 - Fall 2016
*/
#include "fs.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

void fs_stat(file_system *fs, char *path) {
  inode *node = get_inode(fs, path);
  if (!node)
    return;
  printf("  File: '%s'\t", path);
  if (is_directory(node)) {
    printf("Type: Directory\n");
  } else {
    printf("Type: Regular File\n");
  }

  printf("Blocks: %zu\t",
         (node->size + sizeof(data_block) - 1) / sizeof(data_block));
  printf("Links: %d\t", node->hard_link_count);
  printf("Size: %zu\n", node->size);

  printf("Access: %o\t", node->permissions & 0777);
  printf("Uid: %d\t\t", node->owner);
  printf("Gid: %d\n", node->group);

  char buffer[100];
  strftime(buffer, 100, "%F %T %z", localtime(&node->last_access));
  printf("Access: %s\n", buffer);
  strftime(buffer, 100, "%F %T %z", localtime(&node->last_modification));
  printf("Modify: %s\n", buffer);
  strftime(buffer, 100, "%F %T %z", localtime(&node->last_change));
  printf("Change: %s\n", buffer);
}
