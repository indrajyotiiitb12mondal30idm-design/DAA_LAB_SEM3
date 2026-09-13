#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int build_strategy(int n, int *shots) {
    if (n <= 0) return 0;
    if (n == 1) { shots[0] = 1; return 1; }
    if (n == 2) { shots[0] = 1; shots[1] = 1; return 2; }

    int m = 0;
    for (int p = 2; p <= n - 1; p++) shots[m++] = p;     
    if (n % 2 == 0) {
        for (int p = n - 1; p >= 2; p--) shots[m++] = p;  
    } else {
        for (int p = 2; p <= n - 1; p++) shots[m++] = p;  
    }
    return m;
}

int main(int argc, char *argv[]) {
    int n = (argc > 1) ? atoi(argv[1]) : 10;
    if (n < 1) { printf("n must be >= 1\n"); return 1; }

    int *shots = malloc(sizeof(int) * (2 * n + 5));
    int m = build_strategy(n, shots);

    int expected = (n >= 3) ? 2 * (n - 2) : (n == 2 ? 2 : 1);
    printf("Hiding spots (n)         : %d\n", n);
    printf("Shots built by strategy  : %d\n", m);
    printf("Known-optimal formula    : %d\n", expected);
    printf("Match?                   : %s\n", m == expected ? "YES" : "NO");

    char *U = malloc(n + 2), *V = malloc(n + 2), *Unext = malloc(n + 2);
    for (int p = 1; p <= n; p++) U[p] = 1;      
    int guaranteedHitAt = -1;
    for (int t = 0; t < m; t++) {
        int s = shots[t];
        memcpy(V, U, n + 2);
        V[s] = 0;
        int anySurvivor = 0;
        for (int p = 1; p <= n; p++) if (V[p]) anySurvivor = 1;
        if (!anySurvivor) { guaranteedHitAt = t + 1; break; }
        memset(Unext, 0, n + 2);
        for (int p = 1; p <= n; p++) {
            if (!V[p]) continue;
            if (p == 1) Unext[2] = 1;
            else if (p == n) Unext[n - 1] = 1;
            else { Unext[p - 1] = 1; Unext[p + 1] = 1; }
        }
        memcpy(U, Unext, n + 2);
    }

    printf("Belief set becomes empty after shot #: %d (of %d fired)\n",
           guaranteedHitAt, m);
    printf("Validation                : %s\n",
           (guaranteedHitAt == m) ? "PASSED (every possible target is guaranteed hit)"
                                  : "FAILED");

    if (n <= 20) {
        printf("\nShot sequence: ");
        for (int i = 0; i < m; i++) printf("%d ", shots[i]);
        printf("\n");
    }

    free(shots); free(U); free(V); free(Unext);
    return 0;
}

/*
Time Complexity: Theta(n^2)
Space Complexity: Theta(n)
*/