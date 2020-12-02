// RUN: 3c -alltypes %s -- | FileCheck -match-full-lines -check-prefixes="CHECK_ALL","CHECK" %s
// RUN: 3c %s -- | FileCheck -match-full-lines -check-prefixes="CHECK_NOALL","CHECK" %s
// RUN: 3c %s -- | %clang -c -fcheckedc-extension -x c -o /dev/null -

struct { 
    int *data; 
} *x; 
//CHECK_NOALL: _Ptr<int> data;
//CHECK_ALL: _Ptr<struct> x = ((void *)0);

/*ensure trivial conversion*/
void foo(int *x) { 

} 

