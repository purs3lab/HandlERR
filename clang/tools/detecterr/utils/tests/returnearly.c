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
    // should NOT be included
    if(!x){
        return;
    }

    // should NOT be included
    if(x == NULL){
        return;
    }

    // should NOT be included
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


// from libpng
void png_set_write_fn(int *png_ptr, int size){
    // should NOT be included
   if (png_ptr == NULL)
      return;

    printf("size: %d\n", size);
}


typedef int * png_structrp;
typedef int * png_inforp;

void
png_read_info(png_structrp png_ptr, png_inforp info_ptr)
{
   int keep;

   png_debug(1, "in png_read_info");

   if (png_ptr == NULL || info_ptr == NULL)
      return;

   /* Read and check the PNG file signature. */
   png_read_sig(png_ptr, info_ptr);
}
