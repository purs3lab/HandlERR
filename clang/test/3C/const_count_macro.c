// RUN: rm -rf %t*
// RUN: 3c -base-dir=%S -alltypes -addcr %s -- | FileCheck -match-full-lines -check-prefixes="CHECK_ALL" %s
// RUN: 3c -base-dir=%S -alltypes -addcr %s -- | %clang -c -x c -o /dev/null -
// RUN: 3c -base-dir=%S -alltypes -output-dir=%t.checked %s --
// RUN: 3c -base-dir=%t.checked -alltypes %t.checked/const_count_macro.c -- | diff %t.checked/const_count_macro.c -

// Macros used to define the size of an array should be also be used in inferred bounds.

#define SIZE 10

int *test0(int a[SIZE]) {
  int *b = a;
  int *c = b;
  return c;
}
//CHECK_ALL: _Array_ptr<int> test0(int a _Checked[SIZE]) : count(SIZE) _Checked {
//CHECK_ALL: _Array_ptr<int> b : count(SIZE) = a;
//CHECK_ALL: _Array_ptr<int> c : count(SIZE) = b;

int *test1(int *a : count(SIZE)) {
  int *b = a;
  int *c = b;
  return c;
}
//CHECK_ALL: _Array_ptr<int> test1(_Array_ptr<int> a : count(SIZE)) : count(SIZE) _Checked {
//CHECK_ALL: _Array_ptr<int> b : count(SIZE) = a;
//CHECK_ALL: _Array_ptr<int> c : count(SIZE) = b;

void wild(int *a);
int *test2(int *a) {
//CHECK_ALL: int *test2(int *a : itype(_Array_ptr<int>) count(SIZE)) : itype(_Array_ptr<int>) count(SIZE) {
  int b[SIZE];
  a = b;
  wild(a);
  (void) a[0];
  return a;
}

// Sometimes this isn't possible. In this case, the left and right bracket are
// part of the macro, so we can't get just the inner size expression.

#define BAD_SIZE [10]

int *test3(int a BAD_SIZE) {
  int *b = a;
  int *c = b;
  return c;
}
//CHECK_ALL: _Array_ptr<int> test3(int a _Checked BAD_SIZE) : count(10) _Checked {
//CHECK_ALL: _Array_ptr<int> b : count(10) = a;
//CHECK_ALL: _Array_ptr<int> c : count(10) = b;

// Also, if the size of the array is omitted even if there is some text between
// the brackets.

int *test4() {
  int a[/*stuff*/] = {1,2,3};
  int *b = a;
  int *c = b;
  return c;
}
//CHECK_ALL: _Array_ptr<int> test4(void) : count(3) _Checked {
//CHECK_ALL: int a _Checked[/*stuff*/] = {1,2,3};
//CHECK_ALL: _Array_ptr<int> b : count(3) = a;
//CHECK_ALL: _Array_ptr<int> c : count(3) = b;


// A bound might be defined by multiple macros. We can fallback to using the
// integer literal.

#define MERGE_SIZE0 10
#define MERGE_SIZE1 10
int *test5() {
  int a[MERGE_SIZE0];
  int b[MERGE_SIZE1];
  int *c;
  c = a;
  c = b;
  return c;
}
//CHECK_ALL: _Array_ptr<int> test5(void) : count(10) _Checked {
//CHECK_ALL: int a _Checked[MERGE_SIZE0];
//CHECK_ALL: int b _Checked[MERGE_SIZE1];
//CHECK_ALL: _Array_ptr<int> c : count(10) = ((void *)0);


// Test with bounds coming from allocator expression instead of constant array

#include<stdlib.h>

int *test6(int *a) {
  a = malloc(sizeof(int) * SIZE);
  (void) a[0];
  return a;
}
//CHECK_ALL: _Array_ptr<int> test6(_Array_ptr<int> a : count(SIZE)) : count(SIZE) {
//CHECK_ALL: a = malloc<int>(sizeof(int) * SIZE);
