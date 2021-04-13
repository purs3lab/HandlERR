// Tests for 3C.
//
// Checks to make sure _Nt_arrrays only contain pointers & integers
//
// : 3c -base-dir=%S -alltypes %s -- | FileCheck -match-full-lines %s
// RUN: 3c -alltypes -base-dir=%S %s -- | %clang_cc1  -fcheckedc-extension -x c -
// expected-no-diagnostics
struct h;

void k(struct h *e) {
  e = "";
}

void (*a)(struct h *) = k;

void l(struct h *f) {
  k(f);
}
