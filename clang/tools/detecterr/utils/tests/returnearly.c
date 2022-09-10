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

void not_early_2(int *x, int b){
    if(!x){
        return;
    }

    if(x == NULL){
        return;
    }

    if(b == 1){
        return;
    }

    int *a = (int *)malloc(sizeof(int));
    *a = *x + 1;
}


// from vfprintf
static void store_int(void *dest, int size, unsigned long long i)
{
    // should NOT be included
	if (!dest) return;
	switch (size) {
	case 1:
		*(char *)dest = i;
		break;
	case 2:
		*(short *)dest = i;
		break;
	case 3:
		*(int *)dest = i;
		break;
	case 4:
		*(long *)dest = i;
		break;
	case 5:
		*(long long *)dest = i;
		break;
	}
}
