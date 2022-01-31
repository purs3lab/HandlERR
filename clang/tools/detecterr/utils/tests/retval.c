#include <malloc.h>

int *baz(int a){
  int *x = NULL;
  if (x != NULL) {
    x = malloc(sizeof(int));
  }
  x = malloc(sizeof(int));
  return x;
}

int *foo(int a) {
  int *x = NULL;
  if (x != NULL) {
    x = malloc(sizeof(int));
  }
  return x;
}

int *bar(int a) {
  int *x = NULL;
  if (!x) {
    x = malloc(sizeof(int));
  }
  return x;
}
