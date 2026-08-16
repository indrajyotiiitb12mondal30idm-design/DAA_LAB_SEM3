#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <windows.h>
#else
#include <time.h>
#endif

typedef long long ll;

/* ---------- flat n x n matrix helpers (for naive + expansion) ---------- */
ll **allocFlat(int n) {
    ll **m = malloc(n * sizeof(ll *));
    for (int i = 0; i < n; i++) m[i] = calloc(n, sizeof(ll));
    return m;
}
void freeFlat(ll **m, int n) { for (int i = 0; i < n; i++) free(m[i]); free(m); }

void naiveFullMultiply(ll **A, ll **B, ll **C, int n) {
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++) {
            ll s = 0;
            for (int k = 0; k < n; k++) s += A[i][k] * B[k][j];
            C[i][j] = s;
        }
}

/* ---------- compact recursive representation of a "special" matrix ---------- */
typedef struct SpecialMat {
    int n;                     /* size of the FULL matrix this represents */
    ll value;                  /* used only when n == 1 (a plain scalar)  */
    struct SpecialMat *M1, *M2;/* used only when n  > 1                   */
} SpecialMat;

SpecialMat *newLeaf(ll v) {
    SpecialMat *s = malloc(sizeof(SpecialMat));
    s->n = 1; s->value = v; s->M1 = s->M2 = NULL;
    return s;
}
SpecialMat *newNode(SpecialMat *M1, SpecialMat *M2) {
    SpecialMat *s = malloc(sizeof(SpecialMat));
    s->n = M1->n * 2; s->value = 0; s->M1 = M1; s->M2 = M2;
    return s;
}
void freeSpecial(SpecialMat *s) {
    if (!s) return;
    if (s->n > 1) { freeSpecial(s->M1); freeSpecial(s->M2); }
    free(s);
}

/* random special matrix generator, n = 2^k */
SpecialMat *randomSpecial(int n, int maxVal) {
    if (n == 1) return newLeaf(rand() % maxVal - maxVal / 2);
    SpecialMat *m1 = randomSpecial(n / 2, maxVal);
    SpecialMat *m2 = randomSpecial(n / 2, maxVal);
    return newNode(m1, m2);
}

/* expand a SpecialMat into an explicit flat n x n matrix (top-left
   block = M1, top-right = M2, bottom-left = M2, bottom-right = M1,
   applied recursively) - used to build the ground-truth input and to
   verify correctness of the compact-representation algorithms.       */
void expandInto(SpecialMat *s, ll **out, int rowOff, int colOff) {
    if (s->n == 1) { out[rowOff][colOff] = s->value; return; }
    int h = s->n / 2;
    expandInto(s->M1, out, rowOff,     colOff);       /* top-left     */
    expandInto(s->M2, out, rowOff,     colOff + h);   /* top-right    */
    expandInto(s->M2, out, rowOff + h, colOff);       /* bottom-left  */
    expandInto(s->M1, out, rowOff + h, colOff + h);   /* bottom-right */
}
ll **expandFull(SpecialMat *s) {
    ll **out = allocFlat(s->n);
    expandInto(s, out, 0, 0);
    return out;
}

/* elementwise add/sub of two SpecialMats of the SAME size, producing
   a new SpecialMat - because the special pattern is closed under
   addition/subtraction, the result is guaranteed to be special too. */
SpecialMat *specialAdd(SpecialMat *A, SpecialMat *B) {
    if (A->n == 1) return newLeaf(A->value + B->value);
    return newNode(specialAdd(A->M1, B->M1), specialAdd(A->M2, B->M2));
}
SpecialMat *specialSub(SpecialMat *A, SpecialMat *B) {
    if (A->n == 1) return newLeaf(A->value - B->value);
    return newNode(specialSub(A->M1, B->M1), specialSub(A->M2, B->M2));
}
/* divide every entry by 2 (used after the P*R +/- Q*S trick, and is
   ALWAYS exact - see report for the proof)                          */
SpecialMat *specialHalve(SpecialMat *A) {
    if (A->n == 1) return newLeaf(A->value / 2);
    return newNode(specialHalve(A->M1), specialHalve(A->M2));
}

long long g_ops_simple = 0, g_ops_best = 0;   /* scalar mult+add counters */

/* ---------- (2) SIMPLE  O(n^3): use C21=C12,C22=C11, but multiply
   the n/2 blocks with plain naive multiplication (flat arrays)       ---------- */

SpecialMat *flatToSpecial(ll **M, int n, int rowOff, int colOff) {
    if (n == 1) return newLeaf(M[rowOff][colOff]);
    int h = n / 2;
    SpecialMat *m1 = flatToSpecial(M, h, rowOff, colOff);
    SpecialMat *m2 = flatToSpecial(M, h, rowOff, colOff + h);
    return newNode(m1, m2);
}

/* naive O(n^3) multiply of two general flat matrices, with an
   operation counter (each scalar multiply-add counted once)          */
void naiveFlatMultiplyCounted(ll **A, ll **B, ll **C, int n, long long *counter) {
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++) {
            ll s = 0;
            for (int k = 0; k < n; k++) { s += A[i][k] * B[k][j]; (*counter)++; }
            C[i][j] = s;
        }
}

SpecialMat *simpleMultiply(SpecialMat *A, SpecialMat *B) {
    int n = A->n;
    if (n == 1) { g_ops_simple++; return newLeaf(A->value * B->value); }

    int h = n / 2;
    ll **a1 = expandFull(A->M1), **a2 = expandFull(A->M2);
    ll **b1 = expandFull(B->M1), **b2 = expandFull(B->M2);
    ll **t1 = allocFlat(h), **t2 = allocFlat(h), **c11 = allocFlat(h), **c12 = allocFlat(h);

    naiveFlatMultiplyCounted(a1, b1, t1, h, &g_ops_simple);   /* A1*B1 */
    naiveFlatMultiplyCounted(a2, b2, t2, h, &g_ops_simple);   /* A2*B2 */
    for (int i = 0; i < h; i++) for (int j = 0; j < h; j++) c11[i][j] = t1[i][j] + t2[i][j];

    naiveFlatMultiplyCounted(a1, b2, t1, h, &g_ops_simple);   /* A1*B2 */
    naiveFlatMultiplyCounted(a2, b1, t2, h, &g_ops_simple);   /* A2*B1 */
    for (int i = 0; i < h; i++) for (int j = 0; j < h; j++) c12[i][j] = t1[i][j] + t2[i][j];

    SpecialMat *C1 = flatToSpecial(c11, h, 0, 0);
    SpecialMat *C2 = flatToSpecial(c12, h, 0, 0);

    freeFlat(a1,h); freeFlat(a2,h); freeFlat(b1,h); freeFlat(b2,h);
    freeFlat(t1,h); freeFlat(t2,h); freeFlat(c11,h); freeFlat(c12,h);
    return newNode(C1, C2);
}

/* ---------- (3) BEST  O(n^2): P/Q/R/S recursive trick ---------- */
SpecialMat *bestMultiply(SpecialMat *A, SpecialMat *B) {
    int n = A->n;
    if (n == 1) { g_ops_best++; return newLeaf(A->value * B->value); }

    SpecialMat *P = specialAdd(A->M1, A->M2);   /* P = A1+A2 */
    SpecialMat *Q = specialSub(A->M1, A->M2);   /* Q = A1-A2 */
    SpecialMat *R = specialAdd(B->M1, B->M2);   /* R = B1+B2 */
    SpecialMat *S = specialSub(B->M1, B->M2);   /* S = B1-B2 */

    SpecialMat *U = bestMultiply(P, R);         /* U = P*R  (recursive!) */
    SpecialMat *V = bestMultiply(Q, S);         /* V = Q*S  (recursive!) */

    SpecialMat *sum  = specialAdd(U, V);        /* U+V = 2*C11 */
    SpecialMat *diff = specialSub(U, V);        /* U-V = 2*C12 */
    SpecialMat *C1 = specialHalve(sum);         /* C11 */
    SpecialMat *C2 = specialHalve(diff);        /* C12 */

    freeSpecial(P); freeSpecial(Q); freeSpecial(R); freeSpecial(S);
    freeSpecial(U); freeSpecial(V); freeSpecial(sum); freeSpecial(diff);
    return newNode(C1, C2);
}

/* ---------- verification helpers ---------- */
int flatEqual(ll **A, ll **B, int n) {
    for (int i = 0; i < n; i++) for (int j = 0; j < n; j++) if (A[i][j]!=B[i][j]) return 0;
    return 1;
}
double nowSeconds(void) {
#if defined(_WIN32)
    static LARGE_INTEGER freq = {0};
    LARGE_INTEGER now;
    if (freq.QuadPart == 0) {
        QueryPerformanceFrequency(&freq);
    }
    QueryPerformanceCounter(&now);
    return (double)now.QuadPart / (double)freq.QuadPart;
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
#endif
}

int main(void) {
    srand(11);

    /* ---- 1. Correctness across sizes 1,2,4,...,128 ---- */
    int fails = 0;
    for (int n = 1; n <= 128; n *= 2) {
        SpecialMat *A = randomSpecial(n, 9);
        SpecialMat *B = randomSpecial(n, 9);
        ll **flatA = expandFull(A), **flatB = expandFull(B);
        ll **flatC_truth = allocFlat(n);
        naiveFullMultiply(flatA, flatB, flatC_truth, n);

        SpecialMat *Csimple = simpleMultiply(A, B);
        SpecialMat *Cbest   = bestMultiply(A, B);
        ll **flatC_simple = expandFull(Csimple);
        ll **flatC_best   = expandFull(Cbest);

        int ok1 = flatEqual(flatC_truth, flatC_simple, n);
        int ok2 = flatEqual(flatC_truth, flatC_best, n);
        if (!ok1 || !ok2) { printf("MISMATCH n=%d simpleOK=%d bestOK=%d\n", n, ok1, ok2); fails++; }

        freeFlat(flatA,n); freeFlat(flatB,n); freeFlat(flatC_truth,n);
        freeFlat(flatC_simple,n); freeFlat(flatC_best,n);
        freeSpecial(A); freeSpecial(B); freeSpecial(Csimple); freeSpecial(Cbest);
    }
    printf("Correctness check (n=1..128, powers of 2): %d failures\n\n", fails);

    /* ---- 2. Scalar-operation counts, to verify O(n^3) vs O(n^2) growth ---- */
    FILE *fp = fopen("q5_data.csv", "w");
    if (!fp) { perror("fopen"); return 1; }
    fprintf(fp, "n,simple_ops,best_ops,naive_ops,simple_sec,best_sec,naive_sec\n");

    for (int n = 2; n <= 512; n *= 2) {
        SpecialMat *A = randomSpecial(n, 5);
        SpecialMat *B = randomSpecial(n, 5);

        g_ops_simple = 0; g_ops_best = 0;
        double t0 = nowSeconds();
        SpecialMat *Cs = simpleMultiply(A, B);
        double tSimple = nowSeconds() - t0;

        t0 = nowSeconds();
        SpecialMat *Cb = bestMultiply(A, B);
        double tBest = nowSeconds() - t0;

        long long naiveOps = (long long)n * n * n;   /* classic definition */
        ll **flatA = expandFull(A), **flatB = expandFull(B), **flatC = allocFlat(n);
        t0 = nowSeconds();
        naiveFullMultiply(flatA, flatB, flatC, n);
        double tNaive = nowSeconds() - t0;

        fprintf(fp, "%d,%lld,%lld,%lld,%.6f,%.6f,%.6f\n",
                n, g_ops_simple, g_ops_best, naiveOps, tSimple, tBest, tNaive);
        printf("n=%4d  simple_ops=%10lld  best_ops=%10lld  naive_ops(n^3)=%10lld   "
               "t_simple=%.5fs  t_best=%.5fs  t_naive=%.5fs\n",
               n, g_ops_simple, g_ops_best, naiveOps, tSimple, tBest, tNaive);

        freeFlat(flatA,n); freeFlat(flatB,n); freeFlat(flatC,n);
        freeSpecial(A); freeSpecial(B); freeSpecial(Cs); freeSpecial(Cb);
    }
    fclose(fp);
    return 0;
}