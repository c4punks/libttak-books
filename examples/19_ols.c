#include <stdio.h>
#include <stdlib.h>

static int gcd(int a, int b) {
    a = abs(a); b = abs(b);
    while (b) { int t = a % b; a = b; b = t; }
    return a;
}

/* Two diagonal Latin squares L1 = (k*i + j) mod n, L2 = (m*i + j) mod n
 * are orthogonal, and both diagonals stay Latin, when all of
 * k, m, k-m, k+1, k-1, m+1, m-1 are coprime to n.
 * k = 2, m = 3 satisfies this for every n coprime to 6 (5, 7, 11, ...). */
enum { K = 2, M = 3 };

static int coefficients_ok(int n) {
    const int c[] = { K, M, K - M, K + 1, K - 1, M + 1, M - 1 };
    for (size_t i = 0; i < sizeof c / sizeof c[0]; i++)
        if (gcd(c[i], n) != 1) return 0;
    return 1;
}

/* The LUT is a function: no n*n table is ever stored. */
static int magic_value(int n, int i, int j) {
    int l1 = (K * i + j) % n;
    int l2 = (M * i + j) % n;
    return l1 * n + l2 + 1;          /* unique value in 1 .. n*n */
}

int main(int argc, char **argv) {
    int n = (argc > 1) ? atoi(argv[1]) : 5;
    if (n < 3 || !coefficients_ok(n)) {
        fprintf(stderr, "order %d needs other coefficients\n", n);
        return 1;
    }

    const int target = n * (n * n + 1) / 2;
    int diag = 0, anti = 0, ok = 1;
    printf("Magic square of order %d (target sum %d):\n", n, target);
    for (int i = 0; i < n; i++) {
        int row = 0, col = 0;
        for (int j = 0; j < n; j++) {
            printf("%4d", magic_value(n, i, j));
            row += magic_value(n, i, j);
            col += magic_value(n, j, i);
        }
        putchar('\n');
        diag += magic_value(n, i, i);
        anti += magic_value(n, i, n - 1 - i);
        ok &= (row == target && col == target);
    }
    ok &= (diag == target && anti == target);
    printf("rows, columns and both diagonals: %s\n", ok ? "OK" : "FAIL");
    return ok ? 0 : 1;
}
