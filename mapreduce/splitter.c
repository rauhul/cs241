/**
 * MapReduce
 * CS 241 - Fall 2016
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/** Split the lines in a file into parts with approximately the same
    number of bytes in each chunk.

    For example, to split into 4 chunks:
      ./splitter myfile 4 0 > myfile.chunk0
      ./splitter myfile 4 1 > myfile.chunk1
      ./splitter myfile 4 2 > myfile.chunk2
      ./splitter myfile 4 3 > myfile.chunk3

    If the output files are concatenated in order, the result is
    the same as the input file.

    All chunks except the last one end with a newline.

    Some of the output files may be empty.
*/

void printHelp(char *arg);
size_t fileSize(const char *filename);
void getSubrange(size_t length, int count, int idx, size_t *start_offset,
                 size_t *end_offset);
void write_portion(const char *data, size_t length, int count, int index);

int main(int argc, char **argv) {
  if (argc != 4)
    printHelp(*argv);

  const char *filename = argv[1];
  size_t file_size = fileSize(filename);

  int count, index;
  if (1 != sscanf(argv[2], "%d", &count) || count < 1) {
    fprintf(stderr, "Invalid count: %s\n", argv[2]);
    return 1;
  }

  if (1 != sscanf(argv[3], "%d", &index) || index < 0 || index >= count) {
    fprintf(stderr, "Invalid index: %s\n", argv[3]);
    return 1;
  }

  int fd = open(filename, O_RDONLY);
  if (fd == -1) {
    perror(filename);
    exit(1);
  }

  char *data = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (!data) {
    perror("mapping input file");
    exit(1);
  }

  write_portion(data, file_size, count, index);

  munmap(data, file_size);
  return 0;
}

void printHelp(char *arg) {
  fprintf(stderr,
          "\n  %s <input_file> <count> <index>\n\n"
          "  Splits a text file into approximately equal-sized chunks without\n"
          "  breaking up lines.\n"
          "  <count>: the number of chunks.\n"
          "  <index>: index of chunk to output: 0..(<count>-1).\n\n"
          "  For example, to split into 3 equal portions:\n"
          "    %s myfile 3 0 > myfile.0\n"
          "    %s myfile 3 1 > myfile.1\n"
          "    %s myfile 3 2 > myfile.2\n\n",
          arg, arg, arg, arg);
  exit(1);
}

size_t fileSize(const char *filename) {
  struct stat s;
  if (stat(filename, &s)) {
    fprintf(stderr, "%s not found\n", filename);
    exit(1);
  }

  return s.st_size;
}

void getSubrange(size_t length, int count, int idx, size_t *start_offset,
                 size_t *end_offset) {
  *start_offset = length * idx / count;
  *end_offset = length * (idx + 1) / count;
  if (*end_offset > length)
    *end_offset = length;
}

const char *nextLine(const char *p, const char *end) {
  while (p < end) {
    if (*p == '\n')
      return p + 1;
    p++;
  }

  return end;
}

void write_portion(const char *data, size_t length, int count, int index) {
  size_t start_offset, end_offset;
  const char *start_ptr, *end_ptr;

  // Find the byte offsets for the beginning and end of my range
  getSubrange(length, count, index, &start_offset, &end_offset);

  // find my start offset by seeking ahead to the beginning of the next line
  if (index == 0) {
    start_ptr = data;
  } else {
    start_ptr = nextLine(data + start_offset, data + length);
  }

  // find my end offset the same way
  if (index == count - 1) {
    end_ptr = data + length;
  } else {
    end_ptr = nextLine(data + end_offset, data + length);
  }

  // There's a chance both start_offset and end_offset were in the same
  // line and my output will be empty.
  if (end_ptr > start_ptr) {

    // send my range of data to stdout, and be lazy about checking the
    // byte count
    size_t bytes_written = fwrite(start_ptr, 1, end_ptr - start_ptr, stdout);
    if (bytes_written != (size_t)(end_ptr - start_ptr)) {
      fprintf(stderr, "splitter error: tried to write %llu bytes, wrote %llu\n",
              (long long unsigned)(end_ptr - start_ptr),
              (long long unsigned)bytes_written);
      exit(1);
    }
  }
}
