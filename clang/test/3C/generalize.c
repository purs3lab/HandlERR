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




// Code from vsftpd
#include <stdlib.h>
#include <limits.h>
extern void bug(char*);
extern void die(char*);
void*
vsf_sysutil_malloc(unsigned int size)
{
  void* p_ret;
  /* Paranoia - what if we got an integer overflow/underflow? */
  if (size == 0 || size > INT_MAX)
  {
    bug("zero or big size in vsf_sysutil_malloc");
  }
  p_ret = malloc(size);
  if (p_ret == NULL)
  {
    die("malloc");
  }
  return p_ret;
}

void*
vsf_sysutil_realloc(void* p_ptr, unsigned int size)
{
  void* p_ret;
  if (size == 0 || size > INT_MAX)
  {
    bug("zero or big size in vsf_sysutil_realloc");
  }
  p_ret = realloc(p_ptr, size);
  if (p_ret == NULL)
  {
    die("realloc");
  }
  return p_ret;
}

void
vsf_sysutil_free(void* p_ptr)
{
  if (p_ptr == NULL)
  {
    bug("vsf_sysutil_free got a null pointer");
  }
  free(p_ptr);
}

void run_vsf_sysutil (void) {
  typedef struct {char a; char b;} char_node;
  char_node *node1;
  node1 = vsf_sysutil_malloc(sizeof(*node1));
  node1 = vsf_sysutil_realloc(node1, sizeof(*node1));
  vsf_sysutil_free(node1);
}

