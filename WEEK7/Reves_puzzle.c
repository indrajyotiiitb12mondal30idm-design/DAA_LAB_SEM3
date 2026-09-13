#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#define MAXN 40
long T4[MAXN + 1];
int  bestK[MAXN + 1];

long T3(int n) { return (n <= 0) ? 0 : (1L << n) - 1; }

void build_table(int N) {
    T4[0] = 0;
    if (N >= 1) { T4[1] = 1; bestK[1] = 0; }
    for (int n = 2; n <= N; n++) {
        long best = LONG_MAX; int bk = -1;
        for (int k = 1; k < n; k++) {
            long cand = 2 * T4[k] + T3(n - k);
            if (cand < best) { best = cand; bk = k; }
        }
        T4[n] = best; bestK[n] = bk;
    }
}
typedef struct { int disk, from, to; } Move;
Move *moves;
long  moveCount = 0;

void record(int disk, int from, int to) {
    moves[moveCount].disk = disk;
    moves[moveCount].from = from;
    moves[moveCount].to   = to;
    moveCount++;
}
int nextDiskFromTop; 

void Hanoi3_raw(int n, int from, int to, int via, int *diskIds) {
    if (n == 0) return;
    Hanoi3_raw(n - 1, from, via, to, diskIds);
    record(diskIds[n - 1], from, to);
    Hanoi3_raw(n - 1, via, to, from, diskIds);
}

void Hanoi4_raw(int n, int from, int to, int s1, int s2, int *diskIds) {
    if (n == 0) return;
    if (n == 1) { record(diskIds[0], from, to); return; }
    int k = bestK[n];
    Hanoi4_raw(k, from, s1, to, s2, diskIds);             
    Hanoi3_raw(n - k, from, to, s2, diskIds + k);      
    Hanoi4_raw(k, s1, to, from, s2, diskIds);
}

int main(int argc, char *argv[]) {
    int n = (argc > 1) ? atoi(argv[1]) : 8;
    if (n < 1 || n > MAXN) { printf("use 1<=n<=%d\n", MAXN); return 1; }

    build_table(n);

    printf("Disks (n) = %d\n", n);
    printf("n : T4[n] (4-peg optimum) : bestK[n] : T3[n] (3-peg)\n");
    for (int i = 1; i <= n; i++)
        printf("%2d : %6ld : %6d : %6ld\n", i, T4[i], bestK[i], T3(i));

    moves = malloc(sizeof(Move) * (T4[n] + 5));
    int *diskIds = malloc(sizeof(int) * n);
    for (int i = 0; i < n; i++) diskIds[i] = i + 1;

    Hanoi4_raw(n, 1, 2, 3, 4, diskIds);

    printf("\nTotal moves generated : %ld  (matches T4[%d] = %ld ? %s)\n",
           moveCount, n, T4[n], moveCount == T4[n] ? "YES" : "NO");

    int stacks[5][MAXN]; int top[5] = {0,0,0,0,0};
    for (int i = n; i >= 1; i--) stacks[1][top[1]++] = i;  
    int ok = 1;
    for (long i = 0; i < moveCount; i++) {
        int d = moves[i].disk, f = moves[i].from, t = moves[i].to;
        if (top[f] == 0 || stacks[f][top[f]-1] != d) { ok = 0; printf("BAD move %ld: disk %d not on top of peg %d\n", i, d, f); break; }
        if (top[t] > 0 && stacks[t][top[t]-1] < d) { ok = 0; printf("BAD move %ld: disk %d onto smaller disk on peg %d\n", i, d, t); break; }
        top[f]--; stacks[t][top[t]++] = d;
    }
    if (ok) {
        int finalOk = (top[2] == n);
        for (int i = 0; i < n && finalOk; i++) if (stacks[2][i] != n - i) finalOk = 0;
        printf("Move-legality check      : PASSED (no larger disk ever placed on a smaller one)\n");
        printf("Final configuration check: %s (all %d disks correctly stacked on target peg)\n",
               finalOk ? "PASSED" : "FAILED", n);
    }

    if (n <= 8) {
        printf("\nFull move list:\n");
        for (long i = 0; i < moveCount; i++)
            printf("  move %3ld: disk %2d : peg %d -> peg %d\n",
                   i + 1, moves[i].disk, moves[i].from, moves[i].to);
    }

    free(moves); free(diskIds);
    return 0;
}

/*
Time Complexity: Theta(T_4(n)) or O(2^{sqrt{8n}})
Space Complexity: Theta(T_4(n)) or O(2^{sqrt{8n}})
*/