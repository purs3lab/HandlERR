// RUN: rm -rf %t*
// RUN: 3c -base-dir=%S -alltypes -addcr %s -- | FileCheck -match-full-lines -check-prefixes="CHECK_ALL","CHECK" %s
// RUN: 3c -base-dir=%S -addcr %s -- | FileCheck -match-full-lines -check-prefixes="CHECK_NOALL","CHECK" %s
// RUN: 3c -base-dir=%S -addcr %s -- | %clang -c -fcheckedc-extension -x c -o /dev/null -
// RUN: 3c -base-dir=%S -output-dir=%t.checked %s --
// RUN: 3c -base-dir=%t.checked %t.checked/inline_anon_structs.c -- | diff %t.checked/inline_anon_structs.c -

#include <stdlib.h>

/*This code ensures conversion happens as expected when
an inlinestruct and its associated VarDecl have different locations*/
int valuable;

// When -alltypes is on, the addition of _Checked to the array triggers
// rewriting of the multi-decl, including splitting of the struct definition.
// When it is off, the multi-decl as such is not rewritten, but in either case,
// the fields of the struct are rewritten as appropriate.
static struct foo {
  // When an inline struct definition is separated from variable declarations,
  // if there was a `static` keyword that applied to the variables, we should
  // remove it from the separated struct (where it is not meaningful).
  //CHECK_NOALL: static struct foo {
  //CHECK_ALL: struct foo {
  const char *name;
  // TODO: Why is this affected by alltypes if it's a _Ptr?
  //CHECK_NOALL: const char *name;
  //CHECK_ALL:   _Ptr<const char> name;
  int *p_valuable;
  //CHECK: _Ptr<int> p_valuable;
} array[] = {{"mystery", &valuable}};
// {{...}} in a CHECK directive delimits a regular expression
// (https://llvm.org/docs/CommandGuide/FileCheck.html#filecheck-regex-matching-syntax)
// and there isn't a way to escape that construct itself, so we use a regular
// expression and escape the contents as needed.
//CHECK_NOALL: {{\} array\[\] = \{\{"mystery", &valuable\}\};}}
//CHECK_ALL: {{static struct foo array _Checked\[\] = \{\{"mystery", &valuable\}\};}}

/*This code is a series of more complex tests for inline structs*/
/* a, b, c below all stay as WILD pointers; d can be a _Ptr<...>*/

/* one decl; x rewrites to _Ptr<int> */
struct foo1 {
  int *x;
} * a;
//CHECK:      struct foo1 {
//CHECK-NEXT:   _Ptr<int> x;
//CHECK-NEXT: };
//CHECK-NEXT: _Ptr<struct foo1> a = ((void *)0);

struct baz {
  int *z;
};
//CHECK:      struct baz {
//CHECK-NEXT:   _Ptr<int> z;
//CHECK-NEXT: };
struct baz *d;
//CHECK: _Ptr<struct baz> d = ((void *)0);

struct bad {
  int *y;
} * b, *c;
//CHECK:      struct bad {
//CHECK-NEXT:   int *y;
//CHECK-NEXT: };
//CHECK: _Ptr<struct bad> b = ((void *)0);
//CHECK: _Ptr<struct bad> c = ((void *)0);

/* two decls, y should be converted */
struct bar {
  int *y;
} * e, *f;
//CHECK:      struct bar {
//CHECK-NEXT:   _Ptr<int> y;
//CHECK-NEXT: };
//CHECK: _Ptr<struct bar> e = ((void *)0);
//CHECK: _Ptr<struct bar> f = ((void *)0);

void foo(void) {
  a->x = (void *)0;
  b->y = (int *)5;
  d->z = (void *)0;
}

/*This code tests anonymous structs */
struct {
  //CHECK: struct x_struct_1 {
  /*the fields of the anonymous struct are free to be marked checked*/
  int *data;
  //CHECK_NOALL: int *data;
  //CHECK_ALL: _Array_ptr<int> data : count(4);
} * x;
//CHECK:      };
//CHECK-NEXT: _Ptr<struct x_struct_1> x = ((void *)0);

/*ensure trivial conversion*/
void foo1(int *w) {
  //CHECK: void foo1(_Ptr<int> w) {
  x->data = malloc(sizeof(int) * 4);
  x->data[1] = 4;
}

/*This code tests more complex variable declarations*/
struct alpha {
  int *data;
  //CHECK: _Ptr<int> data;
};
struct alpha *al[4];
//CHECK_NOALL: struct alpha *al[4];
//CHECK_ALL: _Ptr<struct alpha> al _Checked[4] = {((void *)0)};

// The anonymous struct no longer blocks `be` from being converted, but we
// still need -alltypes for the array.
struct {
  int *a;
  //CHECK: _Ptr<int> a;
} * be[4];
//CHECK_NOALL:    } * be[4];
//CHECK_ALL:      };
//CHECK_ALL-NEXT: _Ptr<struct be_struct_1> be _Checked[4] = {((void *)0)};

/*this code checks inline structs withiin functions*/
void foo2(int *x) {
  //CHECK: void foo2(_Ptr<int> x) _Checked {
  struct bar {
    int *x;
  } *y = 0;
  //CHECK:      struct bar {
  //CHECK-NEXT:   _Ptr<int> x;
  //CHECK-NEXT: };
  //CHECK-NEXT: _Ptr<struct bar> y = 0;

  // Now that the multi-decl rewriter supports struct splitting in all cases, we
  // no longer constrain struct fields wild in an attempt to avoid needing to
  // add an initializer to non-pointer variables of the struct type and thereby
  // triggering rewriting.
  struct something {
    int *x;
  } z;
  //CHECK:      struct something {
  //CHECK-NEXT:   _Ptr<int> x;
  //CHECK-NEXT: };
  //CHECK-NEXT: struct something z = {};

  // Ditto with an anonymous struct.
  struct {
    int *x;
  } a;
  //CHECK:      struct a_struct_1 {
  //CHECK-NEXT:   _Ptr<int> x;
  //CHECK-NEXT: };
  //CHECK-NEXT: struct a_struct_1 a = {};

  // If the variable already has an initializer, then there is no initializer
  // addition to trigger rewriting and splitting of the struct.
  struct {
    int *c;
  } b = {};
  //CHECK:      struct {
  //CHECK-NEXT:   _Ptr<int> c;
  //CHECK-NEXT: } b = {};

  // Additional regression tests (only checking compilation with no crash) from
  // https://github.com/correctcomputation/checkedc-clang/pull/497.
  struct {
    int *i;
  } * f;
  struct {
    int *il
  } * g, *h, *i;
}
