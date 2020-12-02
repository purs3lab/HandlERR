// RUN: 3c -alltypes -addcr %s -- | FileCheck -match-full-lines -check-prefixes="CHECK_ALL","CHECK" %s
// RUN: 3c -addcr %s -- | FileCheck -match-full-lines -check-prefixes="CHECK_NOALL","CHECK" %s
// RUN: 3c -addcr %s -- | %clang -c -fcheckedc-extension -x c -o /dev/null -
// RUN: 3c -output-postfix=checked -alltypes %s
// RUN: 3c -alltypes %S/inline_anon_structs.checked.c -- | count 0
// RUN: rm %S/inline_anon_structs.checked.c


/*This code ensures conversion happens as expected when 
an inlinestruct and its associated VarDecl have different locations*/
int valuable;

static struct foo
{
  const char* name;
  int* p_valuable;
}
array[] =
{
  { "mystery", &valuable }
}; 
//CHECK_ALL: static struct foo array _Checked[1] =
//CHECK_NOALL: const char* name; 
//CHECK_NOALL: _Ptr<int> p_valuable;

/*This code is a series of more complex tests for inline structs*/
/* a, b, c below all stay as WILD pointers; d can be a _Ptr<...>*/

 /* one decl; x rewrites to _Ptr<int> */
struct foo1 { int *x; } *a;
	//CHECK_NOALL: struct foo1 { _Ptr<int> x; } *a;
	//CHECK_ALL: _Ptr<struct foo1> a = ((void *)0);

struct baz { int *z; };
	//CHECK: struct baz { _Ptr<int> z; };
struct baz *d;
	//CHECK: _Ptr<struct baz> d = ((void *)0);

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
  b->y = (int *) 5;
  d->z = (void*)0;
} 

/*This code tests anonymous structs */
struct { 
	/*the fields of the anonymous struct are free to be marked checked*/
    int *data;

/* but the actual pointer can't be */
} *x;  
//CHECK_ALL: _Ptr<struct> x = ((void *)0); 
//CHECK_NOALL: int *data;

/*ensure trivial conversion*/
void foo1(int *w) { 
	//CHECK: void foo1(_Ptr<int> w) { 
	x->data = malloc(sizeof(int)*4); 
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

/*be should be made wild in the nonalltypes case, whereas a should be converted*/
struct {
  int *a;
} *be[4];
//CHECK_NOALL: _Ptr<int> a; 
//CHECK_ALL: _Ptr<struct> be _Checked[4] = {((void *)0)}; 

/*this code checks inline structs withiin functions*/
void foo2(int *x) {
	//CHECK: void foo2(_Ptr<int> x) {
  struct bar { int *x; } *y = 0;
	//CHECK: struct bar { _Ptr<int> x; } *y = 0; 

  /*A non-pointer struct without an init will be marked wild*/
  struct something { int *x; } z; 
    //CHECK: struct something { int *x; } z; 
  
  /*so will ones that are anonymous*/
  struct { int *x; } a; 
    //CHECK: struct { int *x; } a; 
  
  /*if it have an initializer, the rewriter won't have trouble*/ 
  struct { int * c; } b = {};
	//CHECK: struct { _Ptr<int> c; } b = {};
} 


