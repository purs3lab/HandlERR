// RUN: 3c -alltypes -addcr %s -- | FileCheck -match-full-lines -check-prefixes="CHECK_ALL","CHECK" %s
// RUN: 3c -addcr %s -- | FileCheck -match-full-lines -check-prefixes="CHECK_NOALL","CHECK" %s
// RUN: 3c -addcr %s -- | %clang -c -fcheckedc-extension -x c -o /dev/null -
// RUN: 3c -output-postfix=checked -alltypes %s
// RUN: 3c -alltypes %S/inlinestruct_difflocs.checked.c -- | count 0
// RUN: rm %S/inlinestruct_difflocs.checked.c

int valuable;

static struct foo
{
  const char* name;
	//CHECK_NOALL: const char* name;
	//CHECK_ALL:   _Ptr<const char> name;
  int* p_valuable;
	//CHECK: _Ptr<int> p_valuable;
}
array[] =
{
  { "mystery", &valuable }
}; 

