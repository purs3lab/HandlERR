// RUN: cd %S
// RUN: 3c -alltypes -addcr -output-dir=%t.checked/base_subdir -warn-all-root-cause %s -- -Xclang -verify=unwritable-expected

#include "../unwritable_rootcauses.h"
#include "../root_cause.c"

int foo(void) { 
  // Create some pointers to get actual numbers in the root cause output
  int *a = ptr;
  int *z = a;
  int *q = f1();
  int *w = arr + 2;

  f2(q); // unwritable-expected-warning {{1 unchecked pointer: Default void* type}}
  
  return *z + *q + *w;
}
