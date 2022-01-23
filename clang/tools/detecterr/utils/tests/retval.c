#include <malloc.h>

int *bar(int a) {
  int *x = NULL;
  if (x != NULL) {
    x = malloc(sizeof(int));
  }
  return x;
}

int *baz(int a) {
  int *x = NULL;
  if (!x) {
    x = malloc(sizeof(int));
  }
  return x;
}
