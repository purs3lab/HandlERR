#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>

void exit_wrapper() { exit(1); }

void wrap2() { exit_wrapper(); }

void ehf1(int a) {
  // should be included
  if (a == 1) {
    exit_wrapper();
  }
}

void ehf2(int a) {
  // should be included
  if (a > 0) {
    // should be included
    if (a % 2 == 0) {
      exit_wrapper();
    };
  }
}

void ehf11(int a) {
  // should be included
  if (a == 1) {
    exit(0);
  }
}

// should be included in EHFCat2 List since name contains "err"
void err_ehf_cat2() { return; }

// should be included in EHFCat2 List as return type is void and the function
// writes to stderr
void mylogger1() { fprintf(stderr, "some error message\n"); }

// should be included in EHFCat2 List as return type is void and the function
// writes to stderr
void mylogger2() { vfprintf(stderr, "some error message\n"); }

// should be included in EHFCat2 List as return type is void and the function
// writes to stderr
void mylogger3() { dprintf(2, "some error message\n"); }

// should be included in EHFCat2 List as return type is void and the function
// writes to stderr
void mylogger4() { fwrite("some error message\n", 10, 1, stderr); }

// should be included in EHFCat2 List as return type is void and the function
// writes to stderr
static void mystaticlogger() { fprintf(stderr, "some error message\n"); }

int *myfunc(int a) {
  // should be included
  if (a == 10) {
    mylogger4();
  }

  // should be included
  if (a == 11) {
    err_ehf_cat2();
  }
}

// this function should be included in EHF Cat 1 list
noreturn void noreturn_func() { printf("hello again\n"); }

// function should be included in EHF Cat 1 list, since its just a wrapper
// around a 'noreturn' function
void noreturn_wrapper() { noreturn_func(); }