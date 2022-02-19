#include <stdlib.h>

void exit_wrapper() { exit(1); }

void non_exit_wrapper() { exit(0); }

void wrap2() { exit_wrapper(); }

void non_wrap2() { non_exit_wrapper(); }

void ehf1(int a) {
  if (a == 1) {
    exit_wrapper();
  }
}

void ehf2(int a) {
  if (a > 0) {
    if (a % 2 == 0) {
      exit_wrapper();
    };
  }
}

void non_ehf1(int a) {
  if (a == 1) {
    exit(0);
  }
}
