#!/usr/bin/python

"""
Python implementation of the Bailey-Borwein-Plouffe digit extraction
formula for pi.

         (inf)   1       4      2      1      1
    pi =  SUM { ---- * (---- - ---- - ---- - ----) }
          k=0   16^k    8k+1   8k+4   8k+5   8k+6

See: http://en.wikipedia.org/wiki/BBP_algorithm

Ed Karrels, June 2012
ed.karrels@gmail.com
"""

import sys, time


def series_float(digitOffset, offset, scalePow):
  """
  Compute one term of the summation, scaled by 16^digitOffset.

  (inf) 2^(4*(digitoffset-i)+scalePow)
   SUM {-------------------------}
   i=0        8*i+offset
  """  

  sum = 0
  p = digitOffset*4 + scalePow
  base = offset
  while 1:
    if p >= 0:
      t = float(pow(2, p, base)) / base
    else:
      t = pow(2.0, p) / base
      if t < 1e-17: break
    sum += t
    if sum > 1.0: sum -= 1.0
    p -= 4
    base += 8

  print 'sum(%d) = 0x%014x' % (offset, (sum * 2**56))
  sys.stdout.flush()
  return sum


def plouffe_float(digitOffset):
  sum = 0

  sum += series_float(digitOffset, 1, 2)  # 4/8k+1
  sum -= series_float(digitOffset, 4, 1)  # 2/8k+4
  sum -= series_float(digitOffset, 5, 0)  # 1/8k+5
  sum -= series_float(digitOffset, 6, 0)  # 1/8k+6

  """
  # this version (scaling the first two terms after rather than
  # during summation) tends to have about 2x the error
  sum += 4*series_float(digitOffset, 1, 0)  # 4/8k+1
  sum -= 2*series_float(digitOffset, 4, 0)  # 2/8k+4
  sum -=   series_float(digitOffset, 5, 0)  # 1/8k+5
  sum -=   series_float(digitOffset, 6, 0)  # 1/8k+6
  """

  sum = sum - int(sum)
  if sum < 0: sum += 1

  return int(sum * 2.0**56)


def series_int(scalePow, offset, resultBits, mod):
  sum = 0
  mask = mod-1
  p = scalePow
  base = offset
  while p >= 0:
    t = (pow(2, p, base) * mod // base) & mask
    sum = (sum + t) & mask
    p -= 4
    base += 8

  # print '  regular sum: %0*x' % (resultBits//4, sum & mask)
  saveSum = sum

  t = mod >> (-p)
  while 1:
    f = (t // base) & mask
    sum += f
    t >>= 4
    if t < base: break
    base += 8


  # print '  tail:        %0*x' % (resultBits//4, (sum - saveSum)&mask)

  sum = sum & mask

  print 'sum(%d) = 0x%0*x' % (offset, resultBits//4, sum)

  return sum


def plouffe_int(digitOffset, resultBits):
  mod = 2**resultBits
  sum = 0

  sum += series_int(digitOffset*4+2, 1, resultBits, mod)  # 4/8k+1
  sum -= series_int(digitOffset*4+1, 4, resultBits, mod)  # 2/8k+4
  sum -= series_int(digitOffset*4,   5, resultBits, mod)  # 1/8k+5
  sum -= series_int(digitOffset*4,   6, resultBits, mod)  # 1/8k+6

  sum = (sum - digitOffset) % mod

  return sum


def main(argv):
  if len(argv) < 1 or len(argv) > 2:
    print """
  plouffe.py <digitOffset> [resultBits]
  Compute hex digits of pi using Plouffe's formula starting at <digitOffset>
    pi = 0x3.243f6a8885...
    set digitOffset=0 to get 243f6a8885...
    set digitOffset=1 to get 43f6a8885a...
  resultBits: size of the result in bits, default=128
     use "float" to use double-precision floating point instead
"""
    return 0

  digitOffset = int(argv[0])
  if len(argv) > 1:
    if argv[1] == 'float':
      resultBits = argv[1]
    else:
      resultBits = int(argv[1])
  else:
    resultBits = 128

  print 'Scale = 16^%d' % digitOffset

  startTime = time.time()
  if resultBits == 'float':
    resultBits = 56
    result = plouffe_float(digitOffset)
  else:
    result = plouffe_int(digitOffset, resultBits)
  endTime = time.time()

  print '\n  %0*x\n' % (resultBits // 4, result)
  print 'time: %.3f sec' % (endTime - startTime)


if __name__ == '__main__':
  sys.exit(main(sys.argv[1:]))
