struct {int *a; int *ab;} *x[4], n;
struct foo {int *a; int *b;} *y, d; 
struct bar {int *a; int *b;}; 
struct bar *z; 

void foo() {
    //struct {int *d;} *f;
} 

/*converts to:


_Ptr<struct {_Ptr<int> a; _Ptr<int> ab;}> x _Checked[4] = {((void *)0)};
struct (anonymous struct at /Users/shilpa-roy/checkedc/checkedc-clang/shilpa_issues_examples/example.c:1:1) n;

_Ptr<struct foo {_Ptr<int> a; _Ptr<int> b;}> y = ((void *)0);
struct foo d;
 
struct bar {_Ptr<int> a; _Ptr<int> b;}; 
_Ptr<struct bar> z = ((void *)0); 

void foo() {
    //struct {int *d;} *f;
} 



*/