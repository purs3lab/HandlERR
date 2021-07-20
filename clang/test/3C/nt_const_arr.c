// RUN: rm -rf %t*
// RUN: 3c -base-dir=%S -addcr %s -- | FileCheck -match-full-lines -check-prefixes="CHECK_NOALL" %s
// RUN: 3c -base-dir=%S -alltypes -addcr %s -- | FileCheck -match-full-lines -check-prefixes="CHECK_ALL" %s
// RUN: 3c -base-dir=%S -alltypes -addcr %s -- | %clang -c -x c -o /dev/null -
// RUN: 3c -base-dir=%S -alltypes -output-dir=%t.checked %s --
// RUN: 3c -base-dir=%t.checked -alltypes %t.checked/nt_const_arr.c -- | diff %t.checked/nt_const_arr.c -

// A _Nt_checked constant sized array declared with size `n` should be treated
// as having bounds `count(n - 1)` durring bounds inference because the null
// terminator is included in the declared length but not in the Checked C
// bounds.

unsigned long strlen(const char *s : itype(_Nt_array_ptr<const char>));

void foo() {
  char foo[5] = "test";
  //CHECK_ALL: char foo _Nt_checked[5] = "test";
  //CHECK_NOALL: char foo[5] = "test";
  char *bar = foo;
  //CHECK_ALL: _Nt_array_ptr<char> bar : count(4) = foo;
  //CHECK_NOALL: char *bar = foo;
  strlen(bar);

  char *baz = foo;
  //CHECK_ALL: _Array_ptr<char> baz : count(4) = foo;
  //CHECK_NOALL: char *baz = foo;
  (void) baz[0];
}

void arr_param(char foo[10]) {
//CHECK_ALL: void arr_param(char foo _Nt_checked[10]) _Checked {
//CHECK_NOALL: void arr_param(char foo[10]) {
  char *bar = foo;
  //CHECK_ALL:_Nt_array_ptr<char> bar : count(9) = foo;
  //CHECK_NOALL: char *bar = foo;
  strlen(bar);
}
