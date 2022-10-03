#include <stdio.h>
#include <stdlib.h>

int *another(int *a) {
    int x = 1;
    // should be included
    if (x != 1) {
        // should NOT be included
        if (*a == 0) {
            return NULL;
        }
    }

    return a;
}

int *yet_another3(int *a) {
    int *x = (int *)malloc(sizeof(int));
    *x = 1;

    // should be included
    if (*x == 0) {
        // should NOT be included
        if (*x == 1) {
            return a;
        }
        return x;
    }
    return NULL;
}

int *yet_another2(int a) {
    int *x = (int *)malloc(sizeof(int));
    *x = 1;

    // should be included
    if (*x == 0) {
        // should be included
        if (*x == 1) {
            return NULL;
        }
        return x;
    }
    return NULL;
}

int *yet_another(int a) {
    int *x = (int *)malloc(sizeof(int));
    *x = 1;

    // should NOT be included
    if (*x == 0) {
        // should be included
        if (*x == 1) {
            return NULL;
        }
    }

    return x;
}

int *foo1(int *a) {
    // should NOT be included
    if (*a != 2) {
        // should NOT be included
        if (*a != 3) {
            // should NOT be included
            if (*a < 0) {
                return a;
            }
        }
    }
    return NULL;
}

int *foo(int *a) {
    // should NOT be included
    if (*a != -2) {
        printf("Hello\n");
        // should NOT be included
        if (*a < 0) {
            return a;
        }
    }
    return NULL;
}

int *bar(int *x) {
    // should NOT be included
    if (x == NULL) {
        return NULL;
    }
    return x;
}

int *foo2(int a, int b) {
    // should NOT be included
    if (a == 0 && b == 0) {
        return (int *)malloc(sizeof(int));
    }
    return NULL;
}

int *foo3(int x) {
    int *a = (int *)malloc(sizeof(int));
    // should NOT be listed
    switch (x) {
    case 1:
        return a;
        break;
    default:
        return NULL;
    }
}

int *bar3(int x) {
    int *a = (int *)malloc(sizeof(int));
    // should NOT be included
    if (x == 0) {
        return NULL;
    }
    return a;
}

