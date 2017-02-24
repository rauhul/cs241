/**
 * Mad Mad Access Pattern
 * CS 241 - Fall 2016
 */
#include "tree.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/*
  Look up a few nodes in the tree and print the info they contain.
  This version uses fseek() and fread() to access the data.

  ./lookup1 <data_file> <word> [<word> ...]
*/

char *data_file;

void search_for_word(FILE *data, int offset, char *word) {
    if (offset < 1) {
        printNotFound(word);
        return;
    }

    BinaryTreeNode node;
    if (fseek(data, offset, SEEK_SET) == -1) {
        formatFail(data_file);
        exit(4);
    }

    fread(&node, sizeof(BinaryTreeNode), 1, data);

    if (fseek(data, offset + sizeof(BinaryTreeNode), SEEK_SET) == -1) {
        formatFail(data_file);
        exit(5);
    }

    char node_word[100];
    if (fgets(node_word, 100, data) == NULL) {
        formatFail(data_file);
        exit(6);
    }

    int result = strcmp(word, node_word);
    if (result == 0) {
        printFound(word, node.count, node.price);
        return;
    } else if (result < 0) {
        search_for_word(data, node.left_child, word);
    } else {
        search_for_word(data, node.right_child, word);
    }
}

int main(int argc, char **argv) {

    // parse args
    if (argc < 3) {
        printArgumentUsage();
        exit(1);
    }

    data_file = argv[1];

    // open file
    FILE *data = fopen(data_file, "r");
    if (data == NULL) {
        openFail(data_file);
        exit(2);
    }

    // check file format
    char prefix[5];
    fread(prefix, 4, 1, data);
    prefix[4] = '\0';

    if (strncmp(prefix, BINTREE_HEADER_STRING, 4) != 0) {
        formatFail(data_file);
        exit(2);
    }

    for (int i = 2; i < argc; i++) {
        search_for_word(data, BINTREE_ROOT_NODE_OFFSET, argv[i]);
    }

    fclose(data);

    exit(0);
}