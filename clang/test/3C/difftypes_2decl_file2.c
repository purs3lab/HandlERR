//RUN: rm -rf %t*
//RUN: not 3c -base-dir=%S -output-dir=%t.checked %s %S/difftypes_2decl_file1.c -- 2>%t.stderr
//RUN: grep -q "merging failed" %t.stderr

// The desired behavior in this case is to fail, so other checks are omitted

// 2 conflicting declarations in different files: file 2 of 2.

int * foo(int, char *);
