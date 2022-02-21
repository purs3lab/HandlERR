#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

void _gcry_fatal_error(int rc, const char *text) { abort(); }

void _gcry_divide_by_zero(void) {
  errno = 33;
  _gcry_fatal_error(0, "divide by zero");
}

void foo(int dsize) {
  // should be included
  switch (dsize) {
  case 0:
    _gcry_divide_by_zero();
    break;
  case 1:
    printf("hello\n");
    break;
  }
}
