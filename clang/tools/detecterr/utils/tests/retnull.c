#include<stdio.h>
int* foo(int a) {
  if (a != -2) {
    if (a<0) {
      return &a;
    }
  }
  return -1;
}
