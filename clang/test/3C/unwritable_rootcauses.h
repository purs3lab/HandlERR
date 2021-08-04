
   
void* ptr; // expected-warning {{2 unchecked pointers: Declaration in non-writable file}}


int arr[] = {1,2,3,4};// expected-warning {{1 unchecked pointer: Declaration in non-writable file}}  // expected-warning {{1 unchecked pointer: Expression in non-writable file}}



void* f1(void); // expected-warning {{1 unchecked pointer: Declaration in non-writable file}}


void f2(void*); // expected-warning {{1 unchecked pointer: Declaration in non-writable file}}


