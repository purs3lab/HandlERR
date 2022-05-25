#include <stdlib.h>

void early(){
  int *m = malloc(sizeof(int));
  // should be included
  if(!m){
    return;
  }

  *m = 10;
  *m *= 2;
  return;
}

void not_early(){
  int *m = malloc(sizeof(int));
  // should NOT be included
  if(!m){
    int a = 10;
    return;
  }

  *m = 10;
  *m *= 2;
  return;
}

int *not_early_2(int *x){
    if(!x){
        return NULL;
    }

    int *a = (int *)malloc(sizeof(int));
    *a = *x + 1;
    return a;
}
