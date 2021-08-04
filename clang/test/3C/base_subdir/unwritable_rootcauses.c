// RUN: cd %S
// RUN: 3c -alltypes -addcr -output-dir=%t.checked/base_subdir -warn-all-root-cause %s -- -Xclang -verify

#include "../unwritable_rootcauses.h"

int foo(void) { 
  int *a = ptr;
  int *z = a;
  int* q = f1();
  int *w = arr + 2;

  f2(q); // expected-warning {{1 unchecked pointer: Default void* type}}
  
  return *z + *q + *w;
}
