/**
 * Mad Mad Access Pattern
 * CS 241 - Fall 2016
 */
#include "tree.h"
#include "utils.h"
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/*
  Look up a few nodes in the tree and print the info they contain.
  This version uses mmap to access the data.

  ./lookup2 <data_file> <word> [<word> ...]
*/

void *data;

void search_for_word(int offset, char *word) {
    if (offset < 1) {
        printNotFound(word);
        return;
    }
    BinaryTreeNode *node = (BinaryTreeNode *) (data + offset);

    int result = strcmp(word, node->word);
    if (result == 0) {
        printFound(word, node->count, node->price);
        return;
    } else if (result < 0) {
        search_for_word(node->left_child, word);
    } else {
        search_for_word(node->right_child, word);
    }
}

int main(int argc, char **argv) {

    // parse args
    if (argc < 3) {
        printArgumentUsage();
        exit(1);
    }

    char *data_file = argv[1];

    // open file
    int fd = open(data_file, O_RDONLY);
    if (fd == -1) {
        openFail(data_file);
        exit(2);
    }

    struct stat sb;
    if (fstat(fd, &sb) == -1) {
        openFail(data_file);
        exit(2);
    }

    if (!S_ISREG (sb.st_mode)) {
        openFail(data_file);
        exit(2);
    }

    data = mmap (0, sb.st_size, PROT_READ, MAP_SHARED, fd, 0);
    if (data == MAP_FAILED) {
        mmapFail(data_file);
        exit(3);
    }

    if (close(fd) == -1) {
        openFail(data_file);
        exit(2);
    }

    if (strncmp(data, BINTREE_HEADER_STRING, 4) != 0) {
        formatFail(data_file);
        exit(2);
    }

    for (int i = 2; i < argc; i++) {
        search_for_word(BINTREE_ROOT_NODE_OFFSET, argv[i]);
    }

    exit(0);
}