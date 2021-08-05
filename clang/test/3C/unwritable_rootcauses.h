
   
void* ptr; // unwritable-expected-warning {{2 unchecked pointers: Declaration in non-writable file}}


int arr[] = {1,2,3,4};// unwritable-expected-warning {{1 unchecked pointer: Declaration in non-writable file}}  // unwritable-expected-warning {{1 unchecked pointer: Expression in non-writable file}}



void* f1(void); // unwritable-expected-warning {{1 unchecked pointer: Declaration in non-writable file}}


void f2(void*); // unwritable-expected-warning {{1 unchecked pointer: Declaration in non-writable file}}


