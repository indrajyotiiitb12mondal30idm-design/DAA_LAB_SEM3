#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <windows.h>
#else
#include <time.h>
#endif

typedef long long ll;

/* ---------- simple dynamic square-matrix helpers ---------- */
ll **allocMat(int n) {
    ll **m = malloc(n * sizeof(ll *));
    for (int i = 0; i < n; i++) m[i] = calloc(n, sizeof(ll));
    return m;
}
void freeMat(ll **m, int n) {
    for (int i = 0; i < n; i++) free(m[i]);
    free(m);
}
void addMat(ll **A, ll **B, ll **C, int n) {
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            C[i][j] = A[i][j] + B[i][j];
}
void subMat(ll **A, ll **B, ll **C, int n) {
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            C[i][j] = A[i][j] - B[i][j];
}

/* ---------- (1) NAIVE O(n^3) ---------- */
void naiveMultiply(ll **A, ll **B, ll **C, int n) {
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++) {
            ll sum = 0;
            for (int k = 0; k < n; k++) sum += A[i][k] * B[k][j];
            C[i][j] = sum;
        }
}

/* ---------- (2) PURE STRASSEN (recurses to n==1) ---------- */
void strassenPure(ll **A, ll **B, ll **C, int n) {
    if (n == 1) { C[0][0] = A[0][0] * B[0][0]; return; }

    int h = n / 2;
    ll **A11=allocMat(h),**A12=allocMat(h),**A21=allocMat(h),**A22=allocMat(h);
    ll **B11=allocMat(h),**B12=allocMat(h),**B21=allocMat(h),**B22=allocMat(h);
    for (int i = 0; i < h; i++)
        for (int j = 0; j < h; j++) {
            A11[i][j]=A[i][j];         A12[i][j]=A[i][j+h];
            A21[i][j]=A[i+h][j];       A22[i][j]=A[i+h][j+h];
            B11[i][j]=B[i][j];         B12[i][j]=B[i][j+h];
            B21[i][j]=B[i+h][j];       B22[i][j]=B[i+h][j+h];
        }

    ll **M1=allocMat(h),**M2=allocMat(h),**M3=allocMat(h),**M4=allocMat(h);
    ll **M5=allocMat(h),**M6=allocMat(h),**M7=allocMat(h);
    ll **T1=allocMat(h),**T2=allocMat(h);

    addMat(A11,A22,T1,h); addMat(B11,B22,T2,h); strassenPure(T1,T2,M1,h);
    addMat(A21,A22,T1,h);                       strassenPure(T1,B11,M2,h);
    subMat(B12,B22,T2,h);                       strassenPure(A11,T2,M3,h);
    subMat(B21,B11,T2,h);                       strassenPure(A22,T2,M4,h);
    addMat(A11,A12,T1,h);                       strassenPure(T1,B22,M5,h);
    subMat(A21,A11,T1,h); addMat(B11,B12,T2,h); strassenPure(T1,T2,M6,h);
    subMat(A12,A22,T1,h); addMat(B21,B22,T2,h); strassenPure(T1,T2,M7,h);

    ll **C11=allocMat(h),**C12=allocMat(h),**C21=allocMat(h),**C22=allocMat(h);
    /* C11 = M1+M4-M5+M7 */
    addMat(M1,M4,T1,h); subMat(T1,M5,T2,h); addMat(T2,M7,C11,h);
    /* C12 = M3+M5 */
    addMat(M3,M5,C12,h);
    /* C21 = M2+M4 */
    addMat(M2,M4,C21,h);
    /* C22 = M1-M2+M3+M6 */
    subMat(M1,M2,T1,h); addMat(T1,M3,T2,h); addMat(T2,M6,C22,h);

    for (int i = 0; i < h; i++)
        for (int j = 0; j < h; j++) {
            C[i][j]=C11[i][j];       C[i][j+h]=C12[i][j];
            C[i+h][j]=C21[i][j];     C[i+h][j+h]=C22[i][j];
        }

    ll **all[] = {A11,A12,A21,A22,B11,B12,B21,B22,M1,M2,M3,M4,M5,M6,M7,
                  T1,T2,C11,C12,C21,C22};
    for (unsigned k = 0; k < sizeof(all)/sizeof(all[0]); k++) freeMat(all[k], h);
}

/* ---------- (3) HYBRID STRASSEN with cutoff ("best" in practice) ---------- */
int CUTOFF = 64;   /* below this size, switch to naive multiplication */

void strassenHybrid(ll **A, ll **B, ll **C, int n) {
    if (n <= CUTOFF) { naiveMultiply(A, B, C, n); return; }

    int h = n / 2;
    ll **A11=allocMat(h),**A12=allocMat(h),**A21=allocMat(h),**A22=allocMat(h);
    ll **B11=allocMat(h),**B12=allocMat(h),**B21=allocMat(h),**B22=allocMat(h);
    for (int i = 0; i < h; i++)
        for (int j = 0; j < h; j++) {
            A11[i][j]=A[i][j];         A12[i][j]=A[i][j+h];
            A21[i][j]=A[i+h][j];       A22[i][j]=A[i+h][j+h];
            B11[i][j]=B[i][j];         B12[i][j]=B[i][j+h];
            B21[i][j]=B[i+h][j];       B22[i][j]=B[i+h][j+h];
        }

    ll **M1=allocMat(h),**M2=allocMat(h),**M3=allocMat(h),**M4=allocMat(h);
    ll **M5=allocMat(h),**M6=allocMat(h),**M7=allocMat(h);
    ll **T1=allocMat(h),**T2=allocMat(h);

    addMat(A11,A22,T1,h); addMat(B11,B22,T2,h); strassenHybrid(T1,T2,M1,h);
    addMat(A21,A22,T1,h);                       strassenHybrid(T1,B11,M2,h);
    subMat(B12,B22,T2,h);                       strassenHybrid(A11,T2,M3,h);
    subMat(B21,B11,T2,h);                       strassenHybrid(A22,T2,M4,h);
    addMat(A11,A12,T1,h);                       strassenHybrid(T1,B22,M5,h);
    subMat(A21,A11,T1,h); addMat(B11,B12,T2,h); strassenHybrid(T1,T2,M6,h);
    subMat(A12,A22,T1,h); addMat(B21,B22,T2,h); strassenHybrid(T1,T2,M7,h);

    ll **C11=allocMat(h),**C12=allocMat(h),**C21=allocMat(h),**C22=allocMat(h);
    addMat(M1,M4,T1,h); subMat(T1,M5,T2,h); addMat(T2,M7,C11,h);
    addMat(M3,M5,C12,h);
    addMat(M2,M4,C21,h);
    subMat(M1,M2,T1,h); addMat(T1,M3,T2,h); addMat(T2,M6,C22,h);

    for (int i = 0; i < h; i++)
        for (int j = 0; j < h; j++) {
            C[i][j]=C11[i][j];       C[i][j+h]=C12[i][j];
            C[i+h][j]=C21[i][j];     C[i+h][j+h]=C22[i][j];
        }

    ll **all[] = {A11,A12,A21,A22,B11,B12,B21,B22,M1,M2,M3,M4,M5,M6,M7,
                  T1,T2,C11,C12,C21,C22};
    for (unsigned k = 0; k < sizeof(all)/sizeof(all[0]); k++) freeMat(all[k], h);
}

/* ---------- padding wrapper: makes Strassen work for ANY n, not just
   powers of 2, by padding with zero rows/columns up to the next
   power of 2, multiplying, then cropping back down.                ---------- */
int nextPow2(int n) { int p = 1; while (p < n) p <<= 1; return p; }

void strassenAnySize(ll **A, ll **B, ll **C, int n, int hybrid) {
    int N = nextPow2(n);
    if (N == n) { /* already a power of 2, no padding needed */
        if (hybrid) strassenHybrid(A, B, C, n);
        else        strassenPure(A, B, C, n);
        return;
    }
    ll **PA = allocMat(N), **PB = allocMat(N), **PC = allocMat(N);
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++) { PA[i][j]=A[i][j]; PB[i][j]=B[i][j]; }
    if (hybrid) strassenHybrid(PA, PB, PC, N);
    else        strassenPure(PA, PB, PC, N);
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++) C[i][j] = PC[i][j];
    freeMat(PA,N); freeMat(PB,N); freeMat(PC,N);
}

/* ---------- test / benchmark harness ---------- */
void fillRandom(ll **M, int n, int maxVal) {
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            M[i][j] = rand() % maxVal - maxVal/2;
}
int matricesEqual(ll **A, ll **B, int n) {
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            if (A[i][j] != B[i][j]) return 0;
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
    srand(7);

    /* ---- 1. Correctness: random sizes, including NON power-of-2 ---- */
    int testSizes[] = {1,2,3,5,6,7,8,9,15,16,17,31,33,50,63,64,65,100,127,130};
    int fails = 0;
    for (unsigned t = 0; t < sizeof(testSizes)/sizeof(testSizes[0]); t++) {
        int n = testSizes[t];
        ll **A=allocMat(n),**B=allocMat(n),**Cn=allocMat(n),**Cs=allocMat(n),**Ch=allocMat(n);
        fillRandom(A,n,20); fillRandom(B,n,20);
        naiveMultiply(A,B,Cn,n);
        strassenAnySize(A,B,Cs,n,0);
        strassenAnySize(A,B,Ch,n,1);
        int ok1 = matricesEqual(Cn,Cs,n);
        int ok2 = matricesEqual(Cn,Ch,n);
        if (!ok1 || !ok2) { printf("MISMATCH at n=%d (pureOK=%d hybridOK=%d)\n", n, ok1, ok2); fails++; }
        freeMat(A,n); freeMat(B,n); freeMat(Cn,n); freeMat(Cs,n); freeMat(Ch,n);
    }
    printf("Correctness check across %zu sizes (incl. non-powers-of-2): %d failures\n\n",
           sizeof(testSizes)/sizeof(testSizes[0]), fails);

    /* ---- 2. Timing comparison for the plot ----
       Pure (uncut) Strassen is only benchmarked up to n=256 - beyond
       that its overhead makes it impractically slow (this fact is
       itself the whole point of the "hybrid" section), and running
       it further would just cost time without adding new insight.
       Naive and Hybrid are benchmarked further, to n=1024, to reveal
       the crossover point where Strassen's better asymptotic
       complexity finally overtakes naive's smaller constant factor. */
    FILE *fp = fopen("q4_data.csv", "w");
    if (!fp) { perror("fopen"); return 1; }
    fprintf(fp, "n,naive_sec,strassen_pure_sec,strassen_hybrid_sec\n");

    int sizesAll[]   = {16,32,64,128,256};
    /* Large sweep restricted to EXACT powers of 2 so that Strassen's
       recursion needs no zero-padding - this keeps the timing curve
       clean and directly comparable (padding a non-power-of-2 size
       up to the next power of 2 would otherwise create artificial
       jumps in the plot, since e.g. n=640 would silently become an
       effective 1024).                                               */
    int sizesLarge[] = {512,1024,2048};

    for (unsigned s = 0; s < sizeof(sizesAll)/sizeof(sizesAll[0]); s++) {
        int n = sizesAll[s];
        ll **A=allocMat(n),**B=allocMat(n),**C=allocMat(n);
        fillRandom(A,n,10); fillRandom(B,n,10);

        double t0 = nowSeconds();
        naiveMultiply(A,B,C,n);
        double naiveTime = nowSeconds() - t0;

        t0 = nowSeconds();
        strassenAnySize(A,B,C,n,0);
        double pureTime = nowSeconds() - t0;

        t0 = nowSeconds();
        strassenAnySize(A,B,C,n,1);
        double hybridTime = nowSeconds() - t0;

        fprintf(fp, "%d,%.6f,%.6f,%.6f\n", n, naiveTime, pureTime, hybridTime);
        printf("n=%4d  naive=%.5fs  strassen_pure=%.5fs  strassen_hybrid=%.5fs\n",
               n, naiveTime, pureTime, hybridTime);

        freeMat(A,n); freeMat(B,n); freeMat(C,n);
    }
    for (unsigned s = 0; s < sizeof(sizesLarge)/sizeof(sizesLarge[0]); s++) {
        int n = sizesLarge[s];
        ll **A=allocMat(n),**B=allocMat(n),**C=allocMat(n);
        fillRandom(A,n,10); fillRandom(B,n,10);

        double t0 = nowSeconds();
        naiveMultiply(A,B,C,n);
        double naiveTime = nowSeconds() - t0;

        t0 = nowSeconds();
        strassenAnySize(A,B,C,n,1);
        double hybridTime = nowSeconds() - t0;

        fprintf(fp, "%d,%.6f,,%.6f\n", n, naiveTime, hybridTime);
        printf("n=%4d  naive=%.5fs  strassen_hybrid=%.5fs  (pure skipped: too slow)\n",
               n, naiveTime, hybridTime);

        freeMat(A,n); freeMat(B,n); freeMat(C,n);
    }
    fclose(fp);
    return 0;
}