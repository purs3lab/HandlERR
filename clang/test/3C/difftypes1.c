//RUN: rm -rf %t*
//RUN: not 3c -base-dir=%S -output-dir=%t.checked %s %S/difftypes_xfail2.c -- 2>%t.stderr
//RUN: grep -q "merging failed for 'foo' due to conflicting types for parameter 1" %t.stderr

// The desired behavior in this case is to fail, so other checks are omitted

_Ptr<int> foo(int, char);
