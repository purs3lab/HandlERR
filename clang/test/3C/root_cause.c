// RUN: 3c -base-dir=%S -alltypes -warn-root-cause %s -- -Xclang -verify -Wno-everything

// This test is unusual in that it checks for the errors in the code

#include <stdlib.h>

void *x; // expected-warning {{1 unchecked pointer: Default void* type}}

void test0() {
  int *a;
  char *b;
  a = b; // expected-warning {{2 unchecked pointers: Cast from char * to int *}}

  int *c;
  (char *)c; // expected-warning {{1 unchecked pointer: Cast from int * to char *}}

  int *e;
  char *f;
  f = (char *)e; // expected-warning {{2 unchecked pointers: Cast from int * to char *}}
}

void test1() {
  int a;
  int *b;
  b = malloc(sizeof(int)); // expected-warning {{Bad pointer type solution}}
  b[0] = 1;

  union u {
    int *a; // expected-warning {{1 unchecked pointer: Union or external struct field encountered}}
    int *b; // expected-warning {{1 unchecked pointer: Union or external struct field encountered}}
  };

  void (*c)(void);
  c++; // expected-warning {{1 unchecked pointer: Pointer arithmetic performed on a function pointer}}

  int *d = malloc(1); // expected-warning {{Unsafe call to allocator function}}
}

// expected-warning@+1 {{1 unchecked pointer: External global variable glob has no definition}}
extern int *glob;

// expected-warning@+1 {{1 unchecked pointer: Unchecked pointer in parameter or return of external function glob_f}}
int *glob_f(void);

void (*f)(void *); // expected-warning {{1 unchecked pointer: Default void* type}}

typedef struct {
  int x;
  float f;
} A, *PA;
// expected-warning@-1 {{0 unchecked pointers: Unable to rewrite a typedef with multiple names}}

// expected-warning@+1 {{Internal constraint for generic function declaration, for which 3C currently does not support re-solving.}}
_Itype_for_any(T) void remember(void *p : itype(_Ptr<T>)) {}
