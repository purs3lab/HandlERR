#include<stdio.h>

int* foo(int a) {
  if (a != -2) {
    printf("Hello\n");
    if (a<0) {
      return &a;
    }
  }
  return 0;
}

int bar(int a){
  if (a != -2) {
    printf("Hello\n");
    if (a<0) {
      return a;
    }
  }
  return 0;
}

int* foo2(int a) {
  while (a != -2) {
    printf("Hello\n");
    if (a<0) {
      return &a;
    }
  }
  return 0;
}
