// RUN: 3c -alltypes %s -- | FileCheck -match-full-lines -check-prefixes="CHECK_ALL","CHECK" %s
// RUN: 3c %s -- | FileCheck -match-full-lines -check-prefixes="CHECK_NOALL","CHECK" %s
// RUN: 3c %s -- | %clang -c -fcheckedc-extension -x c -o /dev/null -

/* a, b, c below all stay as WILD pointers; d can be a _Ptr<...>*/

 /* one decl; x rewrites to _Ptr<int> */
struct foo { int *x; } *a;
//CHECK_NOALL: struct foo { _Ptr<int> x; } *a;
//CHECK_ALL: _Ptr<struct foo> a = ((void *)0);

struct baz { int *z; };
struct baz *d;
//CHECK: struct baz { _Ptr<int> z; };
//CHECK: _Ptr<struct baz> d = ((void *)0);

/* two decls, not one; y stays as int * */
struct bad { int* y; } *b, *c; 
//CHECK_NOALL: struct bad { int* y; } *b, *c;
//CHECK_ALL: _Ptr<struct bad> b = ((void *)0); 
//CHECK_ALL: _Ptr<struct bad> c = ((void *)0);

 /* two decls, y should be converted */
struct bar { int* y; } *e, *f;
//CHECK_NOALL: struct bar { _Ptr<int> y; } *e, *f;
//CHECK_ALL: _Ptr<struct bar> e = ((void *)0);
//CHECK_ALL: _Ptr<struct bar> f = ((void *)0);


void foo(void) {
  a->x = (void*)0;  
  b->y = (void*)0;
  d->z = (void*)0;
  c->y = (int *)5; // forces it to WILD
}

