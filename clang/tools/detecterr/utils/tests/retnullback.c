#include<stdio.h>
int* foo(int a) {
  if (a != -2) {
    printf("Hello\n");
    #ifdef HELLO
    if (a<0) {
      return &a;
    }
    #endif
  }
  return NULL;
}
