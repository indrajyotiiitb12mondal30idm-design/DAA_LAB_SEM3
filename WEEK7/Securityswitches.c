#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int *buildSET(int k, long *outLen) {
    if (k == 1) {
        int *a = malloc(sizeof(int));
        a[0] = 1; *outLen = 1; return a;
    }
    long subLen;
    int *sub = buildSET(k - 1, &subLen);
    long total = 2 * subLen + 1;
    int *res = malloc(sizeof(int) * total);
    memcpy(res, sub, sizeof(int) * subLen);
    res[subLen] = k;
    memcpy(res + subLen + 1, sub, sizeof(int) * subLen);
    free(sub);
    *outLen = total;
    return res;
}

int *buildOFF(int n, long *outLen) {
    if (n == 0) { *outLen = 0; return malloc(1); }
    if (n == 1) {
        int *a = malloc(sizeof(int));
        a[0] = 1; *outLen = 1; return a;
    }
    long len1, len2;
    int *off_n2 = buildOFF(n - 2, &len1);
    int *set_n1 = buildSET(n - 1, &len2);
    long total = len1 + 1 + len2;
    int *res = malloc(sizeof(int) * total);
    memcpy(res, off_n2, sizeof(int) * len1);
    res[len1] = n;
    memcpy(res + len1 + 1, set_n1, sizeof(int) * len2);
    free(off_n2); free(set_n1);
    *outLen = total;
    return res;
}

int can_toggle(int *state, int i /* 1-indexed */) {
    if (i == 1) return 1;
    if (state[i - 2] != 1) return 0;          
    for (int j = 1; j <= i - 2; j++)
        if (state[j - 1] != 0) return 0;      
    return 1;
}

int main(int argc, char *argv[]) {
    int n = (argc > 1) ? atoi(argv[1]) : 8;
    if (n < 0 || n > 26) { printf("use 0<=n<=26\n"); return 1; }

    long formula = (1L << (n + 1)) / 3;
    long len;
    int *seq = buildOFF(n, &len);

    printf("Switches (n)                 : %d\n", n);
    printf("Closed form floor(2^(n+1)/3) : %ld\n", formula);
    printf("Generated move-sequence len  : %ld\n", len);
    printf("Match?                       : %s\n", len == formula ? "YES" : "NO");

    int *state = malloc(sizeof(int) * (n > 0 ? n : 1));
    for (int i = 0; i < n; i++) state[i] = 1;        
    int ok = 1;
    for (long t = 0; t < len; t++) {
        int i = seq[t];
        if (!can_toggle(state, i)) {
            printf("ILLEGAL move #%ld : switch %d not allowed in this state\n", t, i);
            ok = 0; break;
        }
        state[i - 1] ^= 1;
    }
    if (ok) {
        int allOff = 1;
        for (int i = 0; i < n; i++) if (state[i] != 0) allOff = 0;
        printf("Legality check                : PASSED (every move obeyed the rule)\n");
        printf("Final state all OFF?          : %s\n", allOff ? "YES" : "NO");
    }

    if (n <= 6) {
        printf("\nFull move sequence (switch numbers, 1 = rightmost):\n  ");
        for (long t = 0; t < len; t++) printf("%d ", seq[t]);
        printf("\n");
    }

    free(seq); free(state);
    return 0;
}

/*
Time Complexity: Theta(2^n)
Space Complexity: Theta(2^n)
*/