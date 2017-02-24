/**
* Finding Filesystems
* CS 241 - Fall 2016
*/
#include "fs.h"
#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

file_system *open_fs(const char *path) {
  if (!path) {
    return NULL;
  }
  struct stat file_stat;
  int open_flags = O_RDONLY;
  int mmap_flags = PROT_READ;

  int fd = open(path, open_flags);
  if (fd == -1) {
    printf("Failed to open file: %s\n", path);
    exit(1);
  }
  if (fstat(fd, &file_stat) != 0) {
    printf("Failed to stat file: %s\n", path);
    exit(1);
  }
  char *file = mmap(NULL, file_stat.st_size, mmap_flags, MAP_SHARED, fd, 0);
  if (file == (void *)-1) {
    printf("Failed to map: %s\n", path);
    exit(1);
  }
  close(fd);
  superblock *metadata = (void *)file;
  file_system *my_fs = malloc(sizeof(*my_fs));
  my_fs->meta = (void *)file;
  my_fs->inode_root = (void *)(file + sizeof(superblock) + DATA_NUMBER);
  my_fs->data_root = (void *)(my_fs->inode_root + metadata->inode_count);

  return my_fs;
}

void close_fs(file_system **fs_pointer) {
  assert(fs_pointer);
  assert(*fs_pointer);
  superblock *meta = (*fs_pointer)->meta;
  munmap(meta, meta->size);
  free(*fs_pointer);
  *fs_pointer = NULL;
}

void set_data_used(file_system *fs_pointer, data_block_number data_number,
                   int used) {
  used = !!(used); // Gives a hard 1 or 0 prediction
  if (data_number < 0 || data_number >= DATA_NUMBER) {
    return;
  }
  fs_pointer->meta->data_map[data_number] = used;
}

data_block_number get_data_used(file_system *fs_pointer, int data_number) {
  if (data_number < 0 || data_number >= DATA_NUMBER) {
    return -1;
  }
  return fs_pointer->meta->data_map[data_number];
}

inode_number first_unused_inode(file_system *fs_pointer) {
  assert(fs_pointer);

  uint64_t i;
  for (i = 1; i < INODES_NUMBER; ++i) {
    int used = fs_pointer->inode_root[i].hard_link_count;
    if (used == 0) {
      return i;
    }
  }
  return -1;
}

data_block_number first_unused_data(file_system *fs_pointer) {
  assert(fs_pointer);

  uint64_t i;
  for (i = 0; i < DATA_NUMBER; ++i) {
    int used = get_data_used(fs_pointer, i);
    if (!used && rand() % 2) {
      return i;
    }
  }
  return -1;
}

data_block_number add_data_block_to_inode(file_system *fs_pointer,
                                          inode *node) {
  assert(fs_pointer);
  assert(node);

  int i;
  for (i = 0; i < NUM_DIRECT_INODES; ++i) {
    if (node->direct_nodes[i] == -1) {
      data_block_number first_data = first_unused_data(fs_pointer);
      if (first_data == -1) {
        return -1;
      }
      node->direct_nodes[i] = first_data;
      set_data_used(fs_pointer, first_data, 1);
      return first_data;
    }
  }
  return 0;
}

inode_number add_single_indirect_block(file_system *fs_pointer, inode *node) {
  assert(fs_pointer);
  assert(node);

  if (node->single_indirect != UNASSIGNED_NODE)
    return -1;
  inode_number first_inode = first_unused_inode(fs_pointer);
  if (first_inode == -1) {
    return -1;
  }
  node->single_indirect = first_inode;
  node->hard_link_count = 1;
  inode *nody = fs_pointer->inode_root + first_inode;
  nody->hard_link_count = 1;
  nody->permissions = TYPE_SINGLE_INDIRECT << RWX_BITS_NUMBER | 0777;
  nody->single_indirect = UNASSIGNED_NODE;
  int i;
  for (i = 0; i < NUM_DIRECT_INODES; ++i)
    nody->direct_nodes[i] = UNASSIGNED_NODE;
  return first_inode;
}

inode *parent_directory(file_system *fs, char *path, char **filename) {
  assert(fs);
  assert(path);

  int len = strlen(path);
  char *endptr = path + len;
  while (*endptr != '/') {
    endptr--;
  }
  *endptr = '\0';

  if (filename) {
    *filename = endptr + 1;
  }
  inode *nody = get_inode(fs, path);
  return nody;
}

int valid_filename(const char *filename) {
  assert(filename);

  if (*filename == '\0') {
    return 0;
  }
  while (*filename) {
    if (*filename == '/') {
      return 0;
    }
    filename++;
  }
  return 1;
}

void init_inode(inode *parent, inode *init) {
  assert(parent);
  assert(init);

  init->owner = parent->owner;
  init->group = parent->group;
  init->permissions =
      (TYPE_FILE << RWX_BITS_NUMBER) | (parent->permissions & 0777);
  init->hard_link_count = 1;
  time(&init->last_access);
  time(&init->last_modification);
  time(&init->last_change);
  init->size = 0;
  init->single_indirect = UNASSIGNED_NODE;
  int i;
  for (i = 0; i < NUM_DIRECT_INODES; ++i)
    init->direct_nodes[i] = UNASSIGNED_NODE;
}

inode *find_inode_named(file_system *fs, inode *root, const char *name) {
  assert(fs);
  assert(root);
  assert(name);

  uint64_t size = root->size;
  int count = 0;
  dirent node;
  while (size > 0 && root->direct_nodes[count] != -1) {
    char *block = (char *)(fs->data_root + root->direct_nodes[count]);
    char *endptr = block + sizeof(data_block);
    while (size > 0 && block < endptr) {
      make_dirent_from_string(block, &node);
      if (strcmp(name, node.name) == 0) {
        return fs->inode_root + node.inode_num;
      }
      block += FILE_NAME_ENTRY;
      size -= FILE_NAME_ENTRY;
    }
    count++;
    if (count == NUM_DIRECT_INODES) {
      if (root->single_indirect == -1) {
        break;
      }
      root = fs->inode_root + root->single_indirect;
      count = 0;
    }
  }
  return NULL;
}

inode *get_inode(file_system *fs, char *path) {
  assert(fs);
  assert(path);

  if (*path == '\0') {
    return fs->inode_root;
  }
  if (*path != '/') {
    return NULL;
  }
  char *path_cpy = strdup(path);
  char *tok = strtok(path_cpy, "/");
  inode *node = fs->inode_root;
  while (node && tok && *tok != '\0') {
    if (!is_directory(node)) {
      node = NULL;
      continue;
    }
    node = find_inode_named(fs, node, tok);
    if (node) {
      tok = strtok(NULL, "/");
    }
  }
  free(path_cpy);
  return node;
}

int is_file(inode *node) {
  return (node->permissions >> RWX_BITS_NUMBER) == TYPE_FILE;
}

int is_directory(inode *node) {
  return (node->permissions >> RWX_BITS_NUMBER) == TYPE_DIRECTORY;
}

int make_dirent_from_string(char *block, dirent *to_fill) {
  char inode_number[9];
  memcpy(inode_number, block + FILE_NAME_LENGTH, 8);
  inode_number[8] = '\0';
  long offset = strtoll(inode_number, NULL, 16);
  if (offset == 0) {
    perror("strtoll:");
    return 0;
  }
  to_fill->inode_num = offset;
  to_fill->name = block;
  return 1;
}