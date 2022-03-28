#include <stdlib.h>

int *foo1(int A) {
  if (A != 2) {
    if (A != 3) {
      if (A < 0) {
        return &A;
      }
    }
  }
  return NULL;
}

int *foo3(int x) {
  int b = 3;
  int *a = malloc(sizeof(int));
  // should be listed
  switch (x) {
  case 1:
    if (b % 2 == 0) {
      return NULL;
    }
    return a;
  default:
    return NULL;
  }
}

int *foo(int *a) {
  if (*a == 1) {
    return (int *)0;
  }

  a = malloc(sizeof(int));

  if (*a > 2) {
    return (int *)0;
  }

  return a;
}
