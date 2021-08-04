// RUN: cd %S
// RUN: 3c -alltypes -addcr -output-dir=%t.checked/base_subdir -warn-all-root-cause %s -- -Xclang -verify

#include "../unwritable_rootcauses.h"

int foo(void) { 
  int *a = ptr;
  int *z = a;
  return *z;
}
