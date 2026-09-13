#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>

void printParens(int **s, int i, int j, char *label) {
    if (i == j) { printf("A%d", i); return; }
    printf("(");
    printParens(s, i, s[i][j], label);
    printParens(s, s[i][j] + 1, j, label);
    printf(")");
}

int main(int argc, char *argv[]) {
    int demo[] = {30, 35, 15, 5, 10, 20, 25};
    int n; int *p;

    if (argc > 1) {
        int count = argc - 1;
        n = count - 1;
        p = malloc(sizeof(int) * count);
        for (int i = 0; i < count; i++) p[i] = atoi(argv[i + 1]);
    } else { n = 6; p = demo; }

    long **m = malloc((n + 1) * sizeof(long *));
    int  **s = malloc((n + 1) * sizeof(int *));
    for (int i = 0; i <= n; i++) {
        m[i] = calloc(n + 1, sizeof(long));
        s[i] = calloc(n + 1, sizeof(int));
    }

    clock_t t0 = clock();
    for (int L = 2; L <= n; L++) {             
        for (int i = 1; i <= n - L + 1; i++) {
            int j = i + L - 1;
            m[i][j] = LONG_MAX;
            for (int k = i; k < j; k++) {
                long c = m[i][k] + m[k+1][j] + (long)p[i-1] * p[k] * p[j];
                if (c < m[i][j]) { m[i][j] = c; s[i][j] = k; }
            }
        }
    }
    clock_t t1 = clock();
    double dpSeconds = (double)(t1 - t0) / CLOCKS_PER_SEC;
    printf("DP fill time (clock())        : %.6f s\n", dpSeconds);

    printf("Matrices (n)                  : %d\n", n);
    printf("Dimensions p[0..n]             : ");
    for (int i = 0; i <= n; i++) printf("%d ", p[i]);
    printf("\n");
    printf("Minimum scalar multiplications : %ld\n", m[1][n]);
    if (n <= 20) {
        printf("Optimal parenthesisation       : ");
        printParens(s, 1, n, "A");
        printf("\n");
    }

    if (n <= 12) {
        printf("\nFull DP table m[i][j] (0 = base case i==j):\n");
        for (int i = 1; i <= n; i++) {
            for (int j = 1; j <= n; j++) {
                if (j < i) printf("%8s", "-");
                else printf("%8ld", m[i][j]);
            }
            printf("\n");
        }
    }

    for (int i = 0; i <= n; i++) { free(m[i]); free(s[i]); }
    free(m); free(s);
    if (argc > 1) free(p);
    return 0;
}



/*
Time Complexity:Theta(n^3)
overall space complexity:Theta(n^2)
*/