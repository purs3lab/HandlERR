// Test that root cause errors are reported correctly in include header files.
// Included by root_cause.c

void undefined(int *p);
// expected-warning@-1 {{1 unchecked pointer: Unchecked pointer in parameter or return of undefined function undefined}}
// unwritable-expected-warning@-2 {{0 unchecked pointers: Source code in non-writable file}}
// unwritable-expected-warning@-3 {{0 unchecked pointers: Unchecked pointer in parameter or return of undefined function undefined}}
