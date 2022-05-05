#include <stdlib.h>

int *baz(int a) {
  int *x = NULL;
  // should not be included
  if (x != NULL) {
    x = malloc(sizeof(int));
  }
  x = malloc(sizeof(int));
  return x;
}

int *foo(int a) {
  int *x = NULL;
  // should be included
  if (x != NULL) {
    x = malloc(sizeof(int));
  }
  return x;
}

int *bar(int a) {
  int *x = NULL;
  // should be included
  if (!x) {
    x = malloc(sizeof(int));
  }
  return x;
}

int *foo_while(int a) {
  int *x = NULL;
  // should be included
  while (x != NULL) {
    x = malloc(sizeof(int));
  }
  return x;
}

int *returns_null(int *i) { return NULL; }

int *foo_check_fn_call(int a) {
  int *x = NULL;
  int *i = malloc(sizeof(int));
  *i = 1;

  // should not be included
  if (!returns_null(i)) {
    x = malloc(sizeof(int));
  }
  return x;
}

int *baz3(int x) {
  int *a = malloc(sizeof(int));
  *a = 0;

  // should not be included
  if (*a == x) {
    *a = 1;
  }
  return a;
}
