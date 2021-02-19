// RUN: rm -rf %t*
// RUN: not 3c -base-dir=%S -output-dir=%t.checked %s %S/multidef1b.c -- 2>%t.stderr
// RUN: grep -q "merging failed" %t.stderr

// The desired behavior in this case is to fail, so other checks are omitted

// Two different definitions of the same function signature, file 1 of 2. This
// tests that 3C rejects duplicate definitions even if the signatures are the
// same.

_Ptr<int> foo(int x, _Ptr<char> y) { 
    x = x + 4; 
    int *z = &x; 
    return z;
}
