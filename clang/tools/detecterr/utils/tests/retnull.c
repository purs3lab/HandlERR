#include <stdio.h>

int *foo1(int a) {
  if (a != 2) {
    if (a != 3) {
    // should not be included
      if (a < 0) {
        return &a;
      }
    }
  }
  return NULL;
}

int *foo(int a) {
  if (a != -2) {
    printf("Hello\n");
    // should not be included
    if (a < 0) {
      return &a;
    }
  }
  return NULL;
}

int *bar(int *x) {
    // should NOT be included
  if (x == NULL) {
    return NULL;
  }
  return x;
}

int *foo2(int a, int b) {
    // should not be included
  if (a == 0 && b == 0) {
    return malloc(sizeof(int));
  }
  return NULL;
}

int *foo3(int x) {
  int *a = malloc(sizeof(int));
  // should not be listed
  switch (x) {
  case 1:
    return a;
    break;
  default:
    return NULL;
  }
}

int *bar3(int x) {
  int *a = malloc(sizeof(int));
  // should be included
  if (x == 0) {
    return NULL;
  }
  return a;
}
