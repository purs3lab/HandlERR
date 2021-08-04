// RUN: rm -rf %t
// RUN: mkdir %t && cd %t
// RUN: python -c 'import json; json.dump([{"arguments": ["clang", "-c", "%s"], "directory": "%S", "file": "%s"}]*2, open("%t/compile_commands.json", "w"))'
// RUN: 3c -p %t -base-dir=%S %s -- | FileCheck -match-full-lines %s
// RUN: 3c -base-dir=%S %s -- | FileCheck -match-full-lines %s

// The compilation database used for this test includes two entries for this
// file, causing 3C to process it twice. In issue #661, this caused type
// argument instantiation to fail.

_Itype_for_any(T) void my_free(void *pointer : itype(_Array_ptr<T>) byte_count(0));

void foo() {
  int *a;
  my_free(a);
  //CHECK: my_free<int>(a);
}
