// RUN: 3c -alltypes -addcr %s -- | FileCheck -match-full-lines -check-prefixes="CHECK_ALL","CHECK" %s
// RUN: 3c -addcr %s -- | FileCheck -match-full-lines -check-prefixes="CHECK_NOALL","CHECK" %s
// RUN: 3c -addcr %s -- | %clang -c -fcheckedc-extension -x c -o /dev/null -
// RUN: 3c -output-postfix=checked -alltypes %s
// RUN: 3c -alltypes %S/anonstruct.checked.c -- | count 0
// RUN: rm %S/anonstruct.checked.c

struct { 
    int *data; 
	//CHECK: _Ptr<int> data; 
} *x; 

/*ensure trivial conversion*/
void foo(int *x) { 
	//CHECK: void foo(_Ptr<int> x) _Checked { 

} 

