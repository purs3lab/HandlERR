// RUN: 3c -alltypes -addcr %s -- | FileCheck -match-full-lines -check-prefixes="CHECK_ALL","CHECK" %s
// RUN: 3c -addcr %s -- | FileCheck -match-full-lines -check-prefixes="CHECK_NOALL","CHECK" %s
// RUN: 3c -alltypes -addcr %s -- | %clang -c -fcheckedc-extension -x c -o /dev/null -
// RUN: 3c -alltypes -output-postfix=checked %s
// RUN: 3c -alltypes %S/liberal_itypes.checked.c -- | count 0
// RUN: rm %S/liberal_itypes.checked.c

void foo(int *a) { }
// CHECK: void foo(_Ptr<int> a) _Checked { }

void bar() {
  int *b = 1;
  // CHECK: int *b = 1;
  foo(b);
  // CHECK: foo(_Assume_bounds_cast<_Ptr<int>>(b));
}

void baz() {
  int *b = 0;
  // CHECK: _Ptr<int> b = 0;
  foo(b);
  // CHECK: foo(b);
}

int *buz() { return 0; }
// CHECK: _Ptr<int> buz(void) _Checked { return 0; }

void boz() {
// CHECK: void boz() {
  int *b  = 1;
  // CHECK: int *b  = 1;
  b = buz();
  // CHECK: b = ((int *)buz());
}

void biz() {
// CHECK: void biz() _Checked {
  int *b  = 0;
  // CHECK: _Ptr<int> b = 0;
  b = buz();
  // CHECK: b = buz();
}

typedef unsigned long size_t;
_Itype_for_any(T) void *malloc(size_t size) : itype(_Array_ptr<T>) byte_count(size);
_Itype_for_any(T) void free(void *pointer : itype(_Array_ptr<T>) byte_count(0));

void malloc_test() {
    int *p = malloc(sizeof(int));
    // CHECK: int *p = malloc<int>(sizeof(int));
    p = 1;
}

void free_test() {
   int *a;
   // CHECK: _Ptr<int> a = ((void *)0);
   free(a);
   // CHECK: free<int>(a);
}

void unsafe(int *a) {
// CHECK: void unsafe(int *a : itype(_Ptr<int>)) {
  a = 1;
}

int *unsafe_return() {
// CHECK: int *unsafe_return(void) : itype(_Ptr<int>) _Checked {
  return 1;
}

void caller() {
  int *b;
  // CHECK: _Ptr<int> b = ((void *)0);
  unsafe(b);
  // CHECK: unsafe(b);

  int *c = 1;
  // CHECK: int *c = 1;
  unsafe(c);
  // CHECK: unsafe(c);

  int *d = unsafe_return();
  // CHECK: _Ptr<int> d = unsafe_return();

  int *e = unsafe_return();
  // CHECK: int *e = unsafe_return();
  e = 1;
}

void fp_test0(int *i) { i = 0; }
// CHECK: void fp_test0(int *i : itype(_Ptr<int>)) { i = 0; }

void fp_test1(int *i) { i = 1; }
// CHECK: void fp_test1(int *i : itype(_Ptr<int>)) { i = 1; }

void fp_caller() {
  void (*j)(int *);
  // CHECK: _Ptr<void (int * : itype(_Ptr<int>))> j = ((void *)0);
  if (0) {
    j = fp_test0;
  } else {
    j = fp_test1;
  }

  int *k;
  j(k);
  // CHECK: _Ptr<int> k = ((void *)0);
  // CHECK: j(k);

  int *l = 1;
  j(l);
  // CHECK: int *l = 1;
  // CHECK: j(l);
}

void fp_test2(int *i) { }
// CHECK: void fp_test2(_Ptr<int> i) _Checked { }
void fp_test3(int *i) { }
// CHECK: void fp_test3(_Ptr<int> i) _Checked { }

void fp_unsafe_caller() {
  void (*a)(int *);
  // CHECK: _Ptr<void (_Ptr<int> )> a = ((void *)0);
  if (0) {
    a = fp_test2;
  } else {
    a = fp_test3;
  }

  int *b = 1;
  // CHECK: int *b = 1;
  a(b);
  // CHECK: a(_Assume_bounds_cast<_Ptr<int>>(b));

  int *c;
  // CHECK: _Ptr<int> c = ((void *)0);
  a(c);
  // CHECK: a(c);
}

int *fp_test4() { return 0; }
// CHECK: int *fp_test4(void) : itype(_Ptr<int>) _Checked { return 0; }
int *fp_test5() { return 1; }
// CHECK: int *fp_test5(void) : itype(_Ptr<int>) _Checked { return 1; }

void fp_unsafe_return() {
  int *(*j)();
  // CHECK: _Ptr<int *(void) : itype(_Ptr<int>)> j = ((void *)0);
  if (0) {
    j = fp_test4;
  } else {
    j = fp_test5;
  }

  int *k = j();
  // CHECK: _Ptr<int> k = j();
}

void thing1(_Ptr<int> i){ }
// CHECK: void thing1(_Ptr<int> i)_Checked { }

void thing2(int *i : itype(_Ptr<int>)){ i = 1; }
// CHECK: void thing2(int *i : itype(_Ptr<int>)){ i = 1; }

void thing3(int *i : itype(_Ptr<int>)){ i = 0; }
// CHECK: void thing3(int *i : itype(_Ptr<int>))_Checked { i = 0; }

void void_ptr(void *p, void *q) {
// CHECK: void void_ptr(void *p, void *q) {
  p = 1;
}

void f_ptr_arg(int (*f)()) {
// CHECK: void f_ptr_arg(int (*f)()) {
  f = 1;
}

void int_ptr_arg(int *a) {}
// CHECK: void int_ptr_arg(_Ptr<int> a) _Checked {}

void char_ptr_param() {
  int_ptr_arg((char*) 1);
  // CHECK: int_ptr_arg(_Assume_bounds_cast<_Ptr<int>>((char*) 1));

  int *c;
  // CHECK: _Ptr<int> c = ((void *)0);
  int_ptr_arg(c);
  // CHECK: int_ptr_arg(c);
}

void fpnc0(void (*fptr)(void *)) { }
// CHECK: void fpnc0(_Ptr<void (void *)> fptr) { }
void fpnc1(void* p1) {}
// CHECK: void fpnc1(void* p1) {}
void fpnc2() { fpnc0(fpnc1); }
// CHECK: void fpnc2() { fpnc0(fpnc1); }
void fpnc3(void (*fptr)(void *)) { fptr = 1; }
// CHECK: void fpnc3(void (*fptr)(void *)) { fptr = 1; }
void fpnc4(void* p1) {}
// CHECK: void fpnc4(void* p1) {}
void fpnc5() { fpnc3(fpnc4); }
// CHECK: void fpnc5() { fpnc3(fpnc4); }


void bounds_fn(void *b : byte_count(1));
// CHECK: void bounds_fn(void *b : byte_count(1));

void bounds_call(void *p) {
// CHECK: void bounds_call(void *p) {
   bounds_fn(p);
   // CHECK: bounds_fn(p);
}

#define macro_cast(x) macro_cast_fn(x)

void macro_cast_fn(int *y) { }
// CHECK: void macro_cast_fn(_Ptr<int> y) _Checked { }

void macro_cast_caller() {
  int *z = 1;
  // CHECK: int *z = 1;
  macro_cast(z);
  // CHECK: macro_cast(_Assume_bounds_cast<_Ptr<int>>(z));
}

char *unused_return_unchecked();
char *unused_return_checked() {return 0;}
char *unused_return_itype() {return 1;}
char **unused_return_unchecked_ptrptr();
//CHECK: char *unused_return_unchecked();
//CHECK: _Ptr<char> unused_return_checked(void) _Checked {return 0;}
//CHECK: char *unused_return_itype(void) : itype(_Ptr<char>) _Checked {return 1;}
//CHECK: char **unused_return_unchecked_ptrptr();

void dont_cast_unused_return() {
   unused_return_unchecked();
   *unused_return_unchecked();
   (void) unused_return_unchecked();
   //CHECK: unused_return_unchecked();
   //CHECK: *unused_return_unchecked();
   //CHECK: (void) unused_return_unchecked();

   unused_return_checked();
   *unused_return_checked();
   (void) unused_return_checked();
   //CHECK: unused_return_checked();
   //CHECK: *unused_return_checked();
   //CHECK: (void) unused_return_checked();

   unused_return_itype();
   *unused_return_itype();
   (void) unused_return_itype();
   //CHECK: unused_return_itype();
   //CHECK: *unused_return_itype();
   //CHECK: (void) unused_return_itype();

   unused_return_unchecked_ptrptr();
   *unused_return_unchecked_ptrptr();
   (void) unused_return_unchecked_ptrptr();
   //CHECK: unused_return_unchecked_ptrptr();
   //CHECK: *unused_return_unchecked_ptrptr();
   //CHECK: (void) unused_return_unchecked_ptrptr();
}

void ptrptr(int **g) {}
//CHECK: void ptrptr(_Ptr<int *> g) {}
void ptrptr_caller() {
  int **a = 1;
  ptrptr(a);
  //CHECK: int **a = 1;
  //CHECK: ptrptr(_Assume_bounds_cast<_Ptr<int *>>(a));

  int **b = 0;
  *b = 1;
  ptrptr(b);
  //CHECK: _Ptr<int *> b = 0;
  //CHECK: ptrptr(b);

  int **c = 0;
  ptrptr(c);
  //CHECK: _Ptr<int *> c = 0;
  //CHECK: ptrptr(c);

  int *d;
  ptrptr(&d);
  //CHECK: int *d;
  //CHECK: ptrptr(&d);

  int ***e;
  ptrptr(*e);
  //CHECK: _Ptr<_Ptr<int *>> e = ((void *)0);
  //CHECK: ptrptr(*e);

  int ***f = 1;
  ptrptr(*f);
  //CHECK: int ***f = 1;
  //CHECK: ptrptr(_Assume_bounds_cast<_Ptr<int *>>(*f));

  int ***g;
  *g = 1;
  ptrptr(*g);
  //CHECK: _Ptr<int **> g = ((void *)0);
  //CHECK: ptrptr(_Assume_bounds_cast<_Ptr<int *>>(*g));
}

void ptrptr_wild(int **y);
//CHECK: void ptrptr_wild(int **y);
int **ptrptr_ret() { return 0; }
//CHECK: _Ptr<int *> ptrptr_ret(void) { return 0; }
void ptrptr_wild_caller() {
  ptrptr_wild(ptrptr_ret());
  //CHECK: ptrptr_wild(((int **)ptrptr_ret()));
}

void ptrptr_itype(int **a) {
//CHECK: void ptrptr_itype(int **a : itype(_Ptr<int *>)) {
  a = 1;

  int **b = a;
  //CHECK: int **b = a;

  int *c = *a;
  //CHECK: int *c = *a;
}

void ptrptr_itype_caller() {
  int **a;
  *a = 1;
  ptrptr_itype(a);
  //CHECK: _Ptr<int *> a = ((void *)0);
  //CHECK: ptrptr_itype(a);

  int **b = 1;
  ptrptr_itype(b);
  //CHECK: int **b = 1;
  //CHECK: ptrptr_itype(b);

  int **c;
  ptrptr_itype(c);
  //CHECK: _Ptr<int *> c = ((void *)0);
  //CHECK: ptrptr_itype(c);

  int ***d;
  ptrptr_itype(*d);
  //CHECK: _Ptr<_Ptr<int *>> d = ((void *)0);
  //CHECK: ptrptr_itype(*d);

  int *e;
  ptrptr_itype(&e);
  //CHECK: int *e;
  //CHECK: ptrptr_itype(&e);
}

int **ptrptr_ret_bad() { return 1; }
//CHECK: int **ptrptr_ret_bad(void) : itype(_Ptr<int *>) { return 1; }
void ptrptr_other() {
  int **a = ptrptr_ret_bad();
  //CHECK: int **a = ptrptr_ret_bad();
  a = 1;

  int **b = ptrptr_ret_bad();
  //CHECK: _Ptr<int *> b = ptrptr_ret_bad();
  *b = 1;

  int **c = ptrptr_ret_bad();
  //CHECK :_Ptr<int *> c = ptrptr_ret_bad();

  int *d = *ptrptr_ret_bad();
  //CHECK: int *d = *ptrptr_ret_bad();
}

int *nested_callee(char *k) { return 0; }
//CHECK: _Ptr<int> nested_callee(_Ptr<char> k) _Checked { return 0; }
int **nested_callee_ptrptr(char **k) { return 0; }
//CHECK: _Ptr<int *> nested_callee_ptrptr(_Ptr<char *> k) { return 0; }
void nested_caller(void) {
//CHECK: void nested_caller(void) {
  char *a = 1;
  nested_callee(nested_callee(a));
  //CHECK: nested_callee(_Assume_bounds_cast<_Ptr<char>>(nested_callee(_Assume_bounds_cast<_Ptr<char>>(a))));

  nested_callee_ptrptr(nested_callee_ptrptr(1));
  //CHECK: nested_callee_ptrptr(_Assume_bounds_cast<_Ptr<char *>>(nested_callee_ptrptr(_Assume_bounds_cast<_Ptr<char *>>(1))));
}

int fptr_itype(void ((*f)(int *)) : itype(_Ptr<void (_Ptr<int>)>));
//CHECK: int fptr_itype(void ((*f)(int *)) : itype(_Ptr<void (_Ptr<int>)>));

void fptr_itype_test(void) {
    _Ptr<int (_Ptr<int>)> fptr1 = ((void *)0);
    //CHECK: _Ptr<int (_Ptr<int>)> fptr1 = ((void *)0);
    baz(fptr1);
    //CHECK: baz(fptr1);

    int (*fptr2)(int *);
    //CHECK: _Ptr<int (_Ptr<int> )> fptr2 = ((void *)0);
    baz(fptr2);
    //CHECK: baz(fptr2);
}

void itype_defined(int *p : itype(_Ptr<int>)) { }
//CHECK: void itype_defined(int *p : itype(_Ptr<int>)) _Checked { }

void itype_defined_ptrptr(int **p : itype(_Ptr<_Ptr<int>>)) { }
//CHECK: void itype_defined_ptrptr(int **p : itype(_Ptr<_Ptr<int>>)) _Checked { }

void itype_defined_caller() {
  int *a = 1;
  itype_defined(a);
  //CHECK: int *a = 1;
  //CHECK: itype_defined(a);

  int *b;
  itype_defined(b);
  //CHECK: _Ptr<int> b = ((void *)0);
  //CHECK: itype_defined(b);

  int **c = 1;
  itype_defined_ptrptr(c);
  //CHECK: int **c = 1;
  //CHECK: itype_defined_ptrptr(c);

  int **d;
  itype_defined_ptrptr(d);
  //CHECK: _Ptr<_Ptr<int>> d = ((void *)0);
  //CHECK: itype_defined_ptrptr(d);

  int **e;
  *e = 1;
  itype_defined_ptrptr(e);
  //CHECK: _Ptr<int *> e = ((void *)0);
  //CHECK: itype_defined_ptrptr(_Assume_bounds_cast<_Ptr<_Ptr<int>>>(e));
}
