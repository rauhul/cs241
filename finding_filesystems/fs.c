/**
* Finding Filesystems
* CS 241 - Fall 2016
*/
#include "format.h"
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

#define DIR_BLOCK_SIZE 256

void inode_ls(file_system *fs, inode *in, uint64_t size) {
    if (!in)
        return;

    if (size % DIR_BLOCK_SIZE != 0)
        assert(0);

    for (int i = 0; i < NUM_DIRECT_INODES; i++) {
        data_block_number num = in->direct_nodes[i];
        data_block block      = fs->data_root[num];

        dirent d;
        char *dirent_start = (char *) &(block.data);
        while (size > 0 && (dirent_start < block.data + sizeof(data_block)) ) {
            make_dirent_from_string(dirent_start, &d);
            inode *node = fs->inode_root + d.inode_num;
            if (!node) {
                print_no_file_or_directory();
                return;
            } else if (is_file(node)) {
                print_file(d.name);
            } else if (is_directory(node)) {
                print_directory(d.name);
            }
            size -= DIR_BLOCK_SIZE;
            dirent_start += DIR_BLOCK_SIZE;
        }
    }

    if (size > 0) {
        inode_ls(fs, fs->inode_root + in->single_indirect, size);
    }
}

void fs_ls(file_system *fs, char *path) {
    inode *in = get_inode(fs, path);
    if (!in) {
        print_no_file_or_directory();
        return;
    }

    if (is_file(in)) {
        print_file(path);
    } else {
        inode_ls(fs, in, in->size);
    }
}

int min(int a, int b) {
    if (a < b)
        return a;
    else
        return b;
}

void inode_cat(file_system *fs, inode *in, uint64_t size) {
    if (!in)
        return;

    for (int i = 0; i < NUM_DIRECT_INODES; i++) {
        data_block_number num = in->direct_nodes[i];
        data_block block      = fs->data_root[num];

        if (size > 0) {
            int write_c = min(size, sizeof(data_block));
            write(1, &block.data, write_c);
            size -= write_c;
        }
    }

    if (size > 0) {
        inode_cat(fs, fs->inode_root + in->single_indirect, size);
    }
}


void fs_cat(file_system *fs, char *path) {
    inode *in = get_inode(fs, path);
    if (!in) {
        print_no_file_or_directory();
        return;
    }

    inode_cat(fs, in, in->size);
}