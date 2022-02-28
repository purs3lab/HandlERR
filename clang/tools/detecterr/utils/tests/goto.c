#include <stdio.h>

int foo(int a) {
  if (a == 1) {
    goto error;
  } else {
    return 1;
  }
error:
  printf("error\n");
}