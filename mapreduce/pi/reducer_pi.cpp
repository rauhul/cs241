#include <stdio.h>
#include <stdlib.h>
#include "pidigits.h"

extern "C" {
#include "reducer.h"
}


/* Input: strings in the form +0x00022e932f201be9 */

u64 parseValue(const char *s) {
  u64 value;
  
  if (1 != sscanf(s, "%llx", &value)) {
    fprintf(stderr, "Invalid value: %s\n", s);
    exit(1);
  }

  return value;
}


const char * reducer(const char * value1, const char * value2) {
  
  u64 sum = 0;
  char *result = (char*) malloc(20);
  
  sum = parseValue(value1) + parseValue(value2);

  sprintf(result, "0x%016llx", sum);

  return result;
}

MAKE_REDUCER_MAIN(reducer)
