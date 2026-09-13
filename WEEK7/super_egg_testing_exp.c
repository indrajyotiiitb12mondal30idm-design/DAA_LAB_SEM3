#include <stdio.h>
#include <stdlib.h>

int egg_drop_optimal(int E, int F, int *movesUsed) {
    int *prev = calloc(E + 1, sizeof(int));   /* f(m-1, *) */
    int *cur  = calloc(E + 1, sizeof(int));   /* f(m,   *) */

    int m = 0;
    while (cur[E] < F) {
        m++;
        for (int e = 1; e <= E; e++)
            cur[e] = prev[e - 1] + prev[e] + 1;
        cur[0] = 0;
        for (int e = 0; e <= E; e++) prev[e] = cur[e];
    }
    *movesUsed = m;
    int result = m;
    free(prev); free(cur);
    return result;
}

int main(int argc, char *argv[]) {
    int E = (argc > 1) ? atoi(argv[1]) : 2;
    int F = (argc > 2) ? atoi(argv[2]) : 100;

    int m;
    int ans = egg_drop_optimal(E, F, &m);

    printf("Eggs (E)                 : %d\n", E);
    printf("Floors (F)                : %d\n", F);
    printf("Minimum guaranteed drops : %d\n", ans);
    printf("(table built with only %d row-updates of O(E) each -> O(E*m) total)\n", m);
    return 0;
}

/*
Time Complexity: Theta(m.E)
Space Complexity: Theta(E)
*/