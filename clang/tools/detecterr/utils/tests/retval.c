#include <malloc.h>

int *baz(int a){
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
