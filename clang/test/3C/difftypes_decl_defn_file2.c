//RUN: rm -rf %t*
//RUN: not 3c -base-dir=%S -output-dir=%t.checked %s %S/difftypes_decl_defn_file1.c -- 2>%t.stderr
//RUN: grep -q "merging failed" %t.stderr

// The desired behavior in this case is to fail, so other checks are omitted

// Conflicting declaration and definition in different files. The RUN lines in
// difftypes_decl_defn_file{1,2}.c test the declaration and definition in both
// orders.

void foo(char **y) {
  // this is the body
}
