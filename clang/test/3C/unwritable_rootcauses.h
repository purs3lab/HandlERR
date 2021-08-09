
   
void* ptr; // unwritable-expected-warning {{2 unchecked pointers: Source code in non-writable file}}

// unwritable-expected-warning@+1 {{1 unchecked pointer: Source code in non-writable file}}  
int arr[] = {1,2,3,4};// unwritable-expected-warning {{1 unchecked pointer: Source code in non-writable file}}



void* f1(void); // unwritable-expected-warning {{1 unchecked pointer: Source code in non-writable file}}


void f2(void*); // unwritable-expected-warning {{1 unchecked pointer: Source code in non-writable file}}


