/**
* Finding Filesystems
* CS 241 - Fall 2016
*/
#pragma once

#include <stdint.h>
#include <time.h>

#define KILOBYTE (1024)

#define INODES_NUMBER (1024)
#define DATA_NUMBER (2 * KILOBYTE)
#define FILE_NAME_LENGTH (248)
#define FILE_NAME_ENTRY (FILE_NAME_LENGTH + 8)
#define UNASSIGNED_NODE (-1)

// Changing default values of enum
enum FILE_TYPE {
  TYPE_FILE = 1,
  TYPE_DIRECTORY = 2,
  TYPE_SINGLE_INDIRECT = 3,
  TYPE_PIPE = 4,
};

#define NUM_DIRECT_INODES 11
#define RWX_BITS_NUMBER 9
#define SEGFAULT *((char *)NULL) = 1;

typedef int data_block_number;
typedef int inode_number;

typedef struct {

  uint64_t size;         /* Size of the mmaped region */
  uint64_t inode_count;  /* Multiple of 64 */
  uint64_t dblock_count; /* Multiple of 64 */
  char data_map[0];      /* Variable sized bitmap */

  /* Do not put anything after this! */
} superblock;
/*
        An inode is set if the num_hard_links is 0.

        After that block we have dblock_count bytes, byte i
                is 0 if that corresponding data block is free

        The superblock is exactly 1 kilobyte in size maximum

        You _do not_ need to worry about this implementation
                this is abstracted away from you!
*/

/*
*	This struct represents an inode
*	It is padded to be perfectly aligned to a 64 byte boundary
*/
typedef struct {

  uint8_t owner; /* Owner ID */
  uint8_t group; /* Group ID */

  /*
  *	Bits 9-0 are rwxrwxrwx
  *	Bits 11-9 are one of directory<d>, file<f>,
  *		character<c>, and pipe<p> in the provided enum
  *		the character and pipe aren't used in this lab
  *		but are there for realism and whatnot
  */
  uint16_t permissions; /* <d,f,c,p>rwxrwxrwx */

  /*
  *	The number of links to the file/directory equals the number
  *		of times it appears in a directory. The inode is considered
  *		a free inode once this count equals 0
  */
  uint32_t hard_link_count; /* reference count, when hit 0 */

  /*
  *	Updated any time any time this inode is read from
  *		For a directory: Every time you've ls'ed or cat'ed directory
  *		For a file: Every time you've cat'ed a file
  */
  time_t last_access;

  /*
  *	Updated any metadata change (links, chmod, ...)
  *		Not needed for you to deal with!
  */
  time_t last_modification;

  /*
  *	Updated any data change
  *		For directories: (mkdir, touch)
  *		For files: cp, other mutation functions
  *		Not needed for you to deal with!
  */
  time_t last_change; /* write change */

  uint64_t size; /* size of the file in bytes */

  /*
  *	Holds the index of the direct data_block
  *
  *	The number of set blocks is continuous
  *		and relates to the size. ceil(size/sizeof(data_block))
  */
  data_block_number direct_nodes[NUM_DIRECT_INODES];
  inode_number single_indirect; /* points to a singly indirect block */
} inode;

typedef struct { char data[16 * KILOBYTE]; } data_block;

typedef struct {
  superblock *meta;  /* Stores the information about system */
  inode *inode_root; /* Pointer to the beginning of an array of inodes */
  data_block
      *data_root; /* Pointer to the beginning of an array of data_blocks */
} file_system;

typedef struct {
  char *name;
  inode_number inode_num;
} dirent;

/*
*	You are going to be implementing this!!!
*	Prints out all the directories located at path
*/
void fs_ls(file_system *fs, char *path);

/*
*	Prints out the file at the at
*	the path (possibly indirect inodes as well)
*/
void fs_cat(file_system *fs, char *path);

/*
*	Prints out the metadata in inode at the end of path
*/
void fs_stat(file_system *fs, char *paths);

/*
*	Takes a valid file-filesystem and opens it
*	While setting up the abstraction
*/
file_system *open_fs(const char *path);

/*
*	Properly writes the file back to disk
*	And stores cleans up the dangling pointer
*/
void close_fs(file_system **fs_pointer);

/*
*	Sets the bytemap after the superblock to the boolean in used
*	No operation if you give an invalid datablock number
*	Used is any valid boolean
*/
void set_data_used(file_system *fs_pointer, data_block_number data_number,
                   int used);

/*
*	Returns whether or not a datablock is being used.
*	No operation if you give an invalid datablock number
*/
int get_data_used(file_system *fs_pointer, data_block_number data_number);

/*
*	Initializes an inode to a file
*	Sets the group, owner, permissions to the parent
*	Hard Link count initialize to 1
*/
void init_inode(inode *parent, inode *init);

/*
*	Returns the inode number of an unused inode
*	-1 if there are no more inodes in the system
*/
inode_number first_unused_inode(file_system *fs_pointer);

/*
*	Returns the data number of an unused inode
*	-1 if there are no more data nodes
*/
data_block_number first_unused_data(file_system *fs_pointer);

/*
*	Gets the inode of the parent directory and the name of the
*	filename at the end (having a trailing slash is undefined behavior)
*	char* path = '/a/b/c'
*	return inode located at '/a/b/c'
*	*filename = 'c'
*/
inode *parent_directory(file_system *fs, char *path, char **filename);

/*
*	Returns 1 if the string is a valid filename
*	Error if passed null
*/
int valid_filename(const char *filename);

/*
*	Adds an indirect block if already not connected to another block
*	Returns -1 if already connected to another block
*/
inode_number add_single_indirect_block(file_system *fs_pointer, inode *node);

/*
*	Adds a data block if not already full
*	Returns -1 on no more data_blocks or if
*	The number of data_blocks is full already
*/
data_block_number add_data_block_to_inode(file_system *fs_pointer, inode *node);

/*********************Important Functions************************/

/*
*	Returns the inode in the filesystem where
*		path = '/path/to/file'
*				'/path/to/dir/'
*				'/path/to/dir'
*		(forward slash on the directory optional)
*	Returns NULL if inode doesn't exist
*/
inode *get_inode(file_system *fs, char *path);

/*
*	Returns true if the inode represents a file
*	Undefined behavior if passed a single indirect block
*/
int is_file(inode *node);

/*
*	Returns true if the inode represents a directory
*	Undefined behavior if passed a single indirect block
*/
int is_directory(inode *node);

/*
*	Accepts a char* ptr to the start of a dirent block
*	char* direct_start points to
*
*	|--------------248--------------||----8----|
*	^
*
*	Undefined behavior if passed anything else.
*	dirent is then filled with the name of the inode
*	and the integer number of said inode
*/
int make_dirent_from_string(char *direct_start, dirent *to_fill);