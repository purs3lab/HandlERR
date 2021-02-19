// RUN: rm -rf %t*
// RUN: not 3c -base-dir=%S -output-dir=%t.checked %s -- 2>%t.stderr
// RUN: grep -q "merging failed" %t.stderr

// The desired behavior in this case is to fail, so other checks are omitted

// 2 conflicting declarations in the same file.

_Ptr<int> foo(int, char);

int * foo(int, char *);
