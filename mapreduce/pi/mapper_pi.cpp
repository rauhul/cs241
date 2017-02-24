/*
  Pi mapper
*/

#include <stdlib.h>
#include <stdio.h>
#include "pidigits.h"

extern "C" {
#include "mapper.h"
}

u64 computeTerms(u64 scalePow, int offset, u64 iterStart, int iters) {

  long long exponent;
  u64 modulo, x, localSum = 0;
    
  exponent = scalePow - 4*iterStart;
  modulo = offset + 8*iterStart;

  for (; iters > 0; iters--) {
    x = CudaPi::modPow2(exponent, modulo);
    // fprintf(stderr, "2^%llu %% %llu = %llu\n", exponent, modulo, x);
    // u64 savex = x;
    x = CudaPi::fraction(x, modulo);
    // fprintf(stderr, "x %016llx / %llu = %015llx\n", savex, modulo, x);
    exponent -= 4;
    modulo += 8;
    localSum += x;
  }
  // fprintf(stderr, "adding %lld..%lld: %016llx\n", iterStart, iterEnd, localSum);

  // master->add(localSum);

  return localSum;
}


u64 computeTail(u64 scalePow, int offset) {
  long long exponent;
  u64 modulo, x, sum = 0;
  u64 numIters = scalePow / 4 + 1;

  exponent = scalePow - 4*numIters;
  modulo = offset + 8*numIters;

  x = 1llu << (64+exponent);
  
  while (x >= modulo) {
    // fprintf(stderr, "  %016llx / %llu\n", x, modulo);
    sum += x / modulo;
    x >>= 4;
    modulo += 8;
  }

  return sum;
}


void mapper(const char *data, FILE *output) {

  int termIdx;  // 0..3
  u64 scalePow, iterStart, target_digit;
  int offset;
  int iters;
  
  if (6 != sscanf(data, "pi_term_%d: digit=%llu scalepow=%llu offset=%d "
		  "start=%llu count=%d",
		  &termIdx, &target_digit, &scalePow, &offset,
		  &iterStart, &iters)) {
    fprintf(stderr, "Bad input format: %s\n", data);
    exit(1);
  }
  
  u64 sum;

  if (iterStart == 0 && iters == 0) {
    sum = computeTail(scalePow, offset);

    // roundoff error correction
    if (termIdx == 0) sum -= target_digit;

  } else {
    sum = computeTerms(scalePow, offset, iterStart, iters);
  }

  char sign = termIdx == 0 ? ' ' : '-';

  fprintf(output, "pi: %c0x%016llx\n", sign, sum);
}

MAKE_MAPPER_MAIN(mapper)
