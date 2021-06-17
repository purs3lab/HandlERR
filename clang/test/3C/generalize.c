// RUN: rm -rf %t*
// RUN: 3c -base-dir=%S -alltypes -addcr %s -- | FileCheck -match-full-lines %s
// RUN: 3c -base-dir=%S -alltypes -addcr %s -- | %clang -c -fcheckedc-extension -x c -o /dev/null -
// RUN: 3c -base-dir=%S -alltypes -output-dir=%t.checked %s --
// RUN: 3c -base-dir=%t.checked -alltypes %t.checked/generalize.c -- | diff %t.checked/generalize.c -

// Test the code that adds generics to replace void*

// Basic functionality
void viewer(void *i) { return; }
// CHECK: _For_any(T) void viewer(_Ptr<T> i) { return; }
void viewer_badnum(void *i, int *j : itype(_Ptr<int>)) {
// CHECK: _Itype_for_any(T) void viewer_badnum(_Ptr<T> i, int *j : itype(_Ptr<int>)) {
j = (int*)3;
return;
}
void *getNull() { return 0; }
// CHECK: _For_any(T) _Ptr<T> getNull(void) { return 0; }

