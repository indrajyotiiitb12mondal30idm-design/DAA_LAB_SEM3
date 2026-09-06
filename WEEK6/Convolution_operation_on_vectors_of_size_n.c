#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <complex.h>

#if defined(_WIN32)
#include <windows.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#else
#include <time.h>
#endif

static double now_sec(void) {
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

/* 
SIMPLE: direct O(n*m) convolution
*/
double* convolve_naive(const double *A, int m, const double *B, int n, int *outLen) {
    int L = n + m - 1;
    double *C = calloc(L, sizeof(double));
    for (int j = 0; j < m; j++) {
        if (A[j] == 0) continue;
        for (int k = 0; k < n; k++)
            C[j + k] += A[j] * B[k];
    }
    *outLen = L;
    return C;
}

/* OPTIMAL SOLUTION */
void fft(double complex *x, int L, int invert) {
    if (L == 1) return;
    double complex *even = malloc(sizeof(double complex) * (L / 2));
    double complex *odd  = malloc(sizeof(double complex) * (L / 2));
    for (int i = 0; i < L / 2; i++) { even[i] = x[2*i]; odd[i] = x[2*i + 1]; }

    fft(even, L / 2, invert);
    fft(odd, L / 2, invert);

    double sign = invert ? 1.0 : -1.0;
    for (int k = 0; k < L / 2; k++) {
        double complex w = cexp(sign * 2.0 * M_PI * I * k / L);
        double complex t = w * odd[k];
        x[k]         = even[k] + t;
        x[k + L / 2] = even[k] - t;
    }
    free(even); free(odd);
}

double* convolve_fft(const double *A, int m, const double *B, int n, int *outLen) {
    int need = n + m - 1;
    int L = 1; while (L < need) L <<= 1;   /* next power of two, no wrap-around */

    double complex *fa = calloc(L, sizeof(double complex));
    double complex *fb = calloc(L, sizeof(double complex));
    for (int i = 0; i < m; i++) fa[i] = A[i];
    for (int i = 0; i < n; i++) fb[i] = B[i];

    fft(fa, L, 0);
    fft(fb, L, 0);
    for (int i = 0; i < L; i++) fa[i] *= fb[i];   /* Convolution Theorem */
    fft(fa, L, 1);

    double *C = malloc(sizeof(double) * need);
    for (int i = 0; i < need; i++) C[i] = creal(fa[i]) / L;  /* undo unnormalised IDFT */

    free(fa); free(fb);
    *outLen = need;
    return C;
}

static void print_vec(const char *label, const double *v, int n) {
    printf("%s [", label);
    for (int i = 0; i < n; i++) printf("%.2f%s", v[i], i + 1 < n ? ", " : "");
    printf("]\n");
}

static void demo(void) {
    double A[] = {1, 2, 3};             /* m = 3 */
    double B[] = {4, 5, 6, 7, 1};       /* n = 5, n >= m as required */
    int m = 3, n = 5;
    print_vec("A =", A, m);
    print_vec("B =", B, n);

    int len1, len2;
    double *c1 = convolve_naive(A, m, B, n, &len1);
    double *c2 = convolve_fft(A, m, B, n, &len2);
    
    printf("\n--- Convolution Results ---\n");
    print_vec("C = A * B (naive)      =", c1, len1);
    print_vec("C = A * B (FFT, D&C)   =", c2, len2);

    double maxerr = 0;
    for (int i = 0; i < len1; i++) maxerr = fmax(maxerr, fabs(c1[i] - c2[i]));
    printf("\nmax |naive - fft| = %.2e  (should be ~1e-9, limited only by floating-point roundoff)\n", maxerr);

    printf("\n--- Theoretical Complexity Analysis ---\n");
    printf("Algorithm: Cooley-Tukey FFT Divide and Conquer\n");
    printf("Time Complexity: O(n log n). Padding to next power of 2 (L) takes O(n). FFT recursive division T(L) = 2T(L/2) + O(L) scales to O(L log L). Since L <= 4n, overall time is O(n log n).\n");
    printf("Space Complexity: O(n) required for L-sized zero-padded complex arrays and O(log n) recursion depth.\n");

    free(c1); free(c2);
}

static void bench(int n) {
    int m = n; /* n >= m required; use m = n for a symmetric timing sweep */
    double *A = malloc(sizeof(double) * m), *B = malloc(sizeof(double) * n);
    srand(1);
    for (int i = 0; i < m; i++) A[i] = rand() % 10;
    for (int i = 0; i < n; i++) B[i] = rand() % 10;

    double t0, t1;
    printf("%d", n);

    int len;
    if ((long)n * m <= 40000000L) { /* cap naive so bench finishes in reasonable time */
        t0 = now_sec(); double *c1 = convolve_naive(A, m, B, n, &len); t1 = now_sec();
        printf(",%.9f", t1 - t0);
        free(c1);
    } else printf(",NaN");

    t0 = now_sec(); double *c2 = convolve_fft(A, m, B, n, &len); t1 = now_sec();
    printf(",%.9f\n", t1 - t0);
    free(c2);

    free(A); free(B);
}

int main(int argc, char **argv) {
    if (argc >= 3 && strcmp(argv[1], "bench") == 0) bench(atoi(argv[2]));
    else demo();
    return 0;
}

/*
Naive Convolution (convolve_naive)----
Time Complexity: O(n × m)
Space/Auxiliary Complexity: O(n + m)


FFT-based Convolution (convolve_fft)----
Time Complexity: O((n + m) log(n + m))
Space/Auxiliary Complexity: O(n + m)  
*/