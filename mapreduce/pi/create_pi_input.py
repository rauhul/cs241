#!/usr/bin/python

import sys

DEFAULT_CHUNK_SIZE = 100000

def printHelp():
  print("""
  create_input <target_digit> [<chunk_size>]
    Create input for a MapReduce job that will compute digits of pi in hex.
    target_digit: index of first digit to find.
      In hex, pi is 3.243f6a88...
      From offset 0: 243f6a88...
      From offset 3: f6a88...
    chunk_size: size of each work chunk
""")
  sys.exit(1)


def main(args):
  if len(args) < 1 or len(args) > 2:
    printHelp()

  target_digit = int(args[0])

  if len(args) > 1:
    chunk_size = int(args[1])
  else:
    chunk_size = DEFAULT_CHUNK_SIZE
  
  printTermChunks(0, target_digit, target_digit * 4 + 2, 1, chunk_size);
  printTermChunks(1, target_digit, target_digit * 4 + 1, 4, chunk_size);
  printTermChunks(2, target_digit, target_digit * 4,     5, chunk_size);
  printTermChunks(3, target_digit, target_digit * 4,     6, chunk_size);


def printTermChunks(term_no, target_digit, scale_pow, offset, chunk_size):
  num_iters = scale_pow / 4 + 1
  
  pos = 0
  while pos < num_iters:
    size = min(chunk_size, num_iters - pos)
    
    print('pi_term_%d: digit=%d scalepow=%d offset=%d start=%d count=%d' %
          (term_no, target_digit, scale_pow, offset, pos, size))

    pos += chunk_size

  # [0,0) marks the end
  print('pi_term_%d: digit=%d scalepow=%d offset=%d start=0 count=0' %
        (term_no, target_digit, scale_pow, offset))


if __name__ == '__main__':
  sys.exit(main(sys.argv[1:]))
