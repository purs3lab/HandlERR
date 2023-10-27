#include <stdio.h>
#include <stdlib.h>

#define ERROR_GUARD if (B == 1)

int B = 2;

int *foo(int a){
  if (a == 1) {
  ERROR_GUARD {
    printf("error\n");
    return malloc(10);
  } 
    exit(1);
   
  }
    return NULL;
}
