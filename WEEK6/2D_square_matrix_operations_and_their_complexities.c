#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#if defined(_WIN32)
#include <windows.h>
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

typedef double* Mat; /* flat n*n array, row-major: M[i*n+j] */

static Mat mat_alloc(int n) { return calloc((size_t)n * n, sizeof(double)); }
static Mat mat_random(int n, int seed) {
    srand(seed);
    Mat m = mat_alloc(n);
    for (int i = 0; i < n * n; i++) m[i] = (rand() % 20) - 10;
    return m;
}
static Mat mat_copy(const Mat a, int n) {
    Mat b = mat_alloc(n);
    memcpy(b, a, sizeof(double) * n * n);
    return b;
}
static void mat_print(const char *label, const Mat a, int n) {
    printf("%s\n", label);
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) printf("%7.2f ", a[i * n + j]);
        printf("\n");
    }
}

/* (i) Matrix addition */

/*
Time complexity: O(n^2)
Space complexity: O(n^2) auxiliary 
*/
Mat mat_add(const Mat a, const Mat b, int n) {
    Mat c = mat_alloc(n);
    for (int i = 0; i < n * n; i++) c[i] = a[i] + b[i];
    return c;
}

/* (ii) Matrix multiplication (Strassen) */

/*
Time complexity: O(n^log2(7)) ~ O(n^2.81)
Space complexity: O(n^2) auxiliary
*/
static Mat mat_add_ex(const Mat a, const Mat b, int n, int sign) {
    Mat c = mat_alloc(n);
    for (int i = 0; i < n * n; i++) c[i] = a[i] + sign * b[i];
    return c;
}
static void mat_free(Mat m) { free(m); }

/* Base case helper for Strassen */
static Mat mat_mul_base(const Mat a, const Mat b, int n) {
    Mat c = mat_alloc(n);
    for (int i = 0; i < n; i++) {
        for (int k = 0; k < n; k++) {
            for (int j = 0; j < n; j++) c[i * n + j] += a[i * n + k] * b[k * n + j];
        }
    }
    return c;
}

static Mat strassen_rec(const Mat a, const Mat b, int n) {
    if (n <= 64) return mat_mul_base(a, b, n); /* base case: crossover point */
    int h = n / 2;
    Mat a11 = mat_alloc(h), a12 = mat_alloc(h), a21 = mat_alloc(h), a22 = mat_alloc(h);
    Mat b11 = mat_alloc(h), b12 = mat_alloc(h), b21 = mat_alloc(h), b22 = mat_alloc(h);
    for (int i = 0; i < h; i++)
        for (int j = 0; j < h; j++) {
            a11[i*h+j] = a[i*n+j];         a12[i*h+j] = a[i*n+j+h];
            a21[i*h+j] = a[(i+h)*n+j];     a22[i*h+j] = a[(i+h)*n+j+h];
            b11[i*h+j] = b[i*n+j];         b12[i*h+j] = b[i*n+j+h];
            b21[i*h+j] = b[(i+h)*n+j];     b22[i*h+j] = b[(i+h)*n+j+h];
        }
    Mat t1 = mat_add_ex(a11, a22, h, 1), t2 = mat_add_ex(b11, b22, h, 1);
    Mat m1 = strassen_rec(t1, t2, h); mat_free(t1); mat_free(t2);

    Mat t3 = mat_add_ex(a21, a22, h, 1);
    Mat m2 = strassen_rec(t3, b11, h); mat_free(t3);

    Mat t4 = mat_add_ex(b12, b22, h, -1);
    Mat m3 = strassen_rec(a11, t4, h); mat_free(t4);

    Mat t5 = mat_add_ex(b21, b11, h, -1);
    Mat m4 = strassen_rec(a22, t5, h); mat_free(t5);

    Mat t6 = mat_add_ex(a11, a12, h, 1);
    Mat m5 = strassen_rec(t6, b22, h); mat_free(t6);

    Mat t7 = mat_add_ex(a21, a11, h, -1), t8 = mat_add_ex(b11, b12, h, 1);
    Mat m6 = strassen_rec(t7, t8, h); mat_free(t7); mat_free(t8);

    Mat t9 = mat_add_ex(a12, a22, h, -1), t10 = mat_add_ex(b21, b22, h, 1);
    Mat m7 = strassen_rec(t9, t10, h); mat_free(t9); mat_free(t10);

    Mat c = mat_alloc(n);
    for (int i = 0; i < h; i++)
        for (int j = 0; j < h; j++) {
            c[i*n+j] = m1[i*h+j] + m4[i*h+j] - m5[i*h+j] + m7[i*h+j];
            c[i*n+j+h] = m3[i*h+j] + m5[i*h+j];
            c[(i+h)*n+j] = m2[i*h+j] + m4[i*h+j];
            c[(i+h)*n+j+h] = m1[i*h+j] - m2[i*h+j] + m3[i*h+j] + m6[i*h+j];
        }
    mat_free(a11); mat_free(a12); mat_free(a21); mat_free(a22);
    mat_free(b11); mat_free(b12); mat_free(b21); mat_free(b22);
    mat_free(m1); mat_free(m2); mat_free(m3); mat_free(m4);
    mat_free(m5); mat_free(m6); mat_free(m7);
    return c;
}

Mat mat_mul_strassen(const Mat a, const Mat b, int n) {
    int m = 1; while (m < n) m <<= 1;
    Mat pa = mat_alloc(m), pb = mat_alloc(m);
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++) { pa[i*m+j] = a[i*n+j]; pb[i*m+j] = b[i*n+j]; }
    Mat pc = strassen_rec(pa, pb, m);
    Mat c = mat_alloc(n);
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++) c[i*n+j] = pc[i*m+j];
    mat_free(pa); mat_free(pb); mat_free(pc);
    return c;
}

/* (iii) Is the matrix a zero matrix? */

/*
Time complexity: O(n^2)
Space complexity: O(1) auxiliary
*/
int is_zero_matrix(const Mat a, int n) {
    for (int i = 0; i < n * n; i++) if (a[i] != 0) return 0;
    return 1;
}

/* (iv) Is the matrix symmetric? (A == A^T) */

/*
Time complexity: O(n^2)
Space complexity: O(1) auxiliary
*/
int is_symmetric_optimal(const Mat a, int n) {
    for (int i = 0; i < n; i++)
        for (int j = i + 1; j < n; j++)
            if (a[i * n + j] != a[j * n + i]) return 0;
    return 1;
}

/* (v) Determinant */

/*
Time complexity: O(n^3)
Space complexity: O(n^2) auxiliary
*/
double det_lu(const Mat a, int n) {
    Mat u = mat_copy(a, n);
    double det = 1.0;
    for (int col = 0; col < n; col++) {
        int piv = col;
        for (int i = col + 1; i < n; i++)
            if (fabs(u[i * n + col]) > fabs(u[piv * n + col])) piv = i;
        if (fabs(u[piv * n + col]) < 1e-12) { mat_free(u); return 0.0; }
        if (piv != col) {
            for (int j = 0; j < n; j++) { double t = u[col*n+j]; u[col*n+j] = u[piv*n+j]; u[piv*n+j] = t; }
            det = -det;
        }
        for (int i = col + 1; i < n; i++) {
            double f = u[i * n + col] / u[col * n + col];
            for (int j = col; j < n; j++) u[i * n + j] -= f * u[col * n + j];
        }
        det *= u[col * n + col];
    }
    mat_free(u);
    return det;
}

/* (vi) Transpose IN SITU (in place) */

/*
Time complexity: O(n^2)
Space complexity: O(1) auxiliary
*/
void transpose_inplace(Mat a, int n) {
    for (int i = 0; i < n; i++)
        for (int j = i + 1; j < n; j++) {
            double t = a[i * n + j]; a[i * n + j] = a[j * n + i]; a[j * n + i] = t;
        }
}

/* (vii) Eigenvalue(s) / eigenvector(s) */

/*
Time complexity: O(k*n^2) for k iterations
Space complexity: O(n) auxiliary
*/
static void matvec(const Mat a, const double *x, double *y, int n) {
    for (int i = 0; i < n; i++) {
        double s = 0;
        for (int j = 0; j < n; j++) s += a[i * n + j] * x[j];
        y[i] = s;
    }
}

double power_iteration(const Mat a, int n, double *eigvec, int iters) {
    double *v = malloc(sizeof(double) * n), *w = malloc(sizeof(double) * n);
    for (int i = 0; i < n; i++) v[i] = 1.0; 
    double lambda = 0;
    for (int it = 0; it < iters; it++) {
        matvec(a, v, w, n);
        double norm = 0;
        for (int i = 0; i < n; i++) norm += w[i] * w[i];
        norm = sqrt(norm);
        for (int i = 0; i < n; i++) v[i] = w[i] / norm;
        lambda = norm; 
    }
    matvec(a, v, w, n);
    double num = 0, den = 0;
    for (int i = 0; i < n; i++) { num += v[i] * w[i]; den += v[i] * v[i]; }
    lambda = num / den;
    if (eigvec) memcpy(eigvec, v, sizeof(double) * n);
    free(v); free(w);
    return lambda;
}

static void demo(void) {
    int n = 4;
    double vals[16] = { 4,1,2,0,  1,3,0,1,  2,0,5,2,  0,1,2,6 }; 
    Mat a = mat_alloc(n);
    memcpy(a, vals, sizeof(vals));
    mat_print("A =", a, n);

    Mat b = mat_random(n, 7);
    mat_print("\nB =", b, n);

    printf("\n--- Complexity Analysis Validation ---\n");

    Mat c = mat_add(a, b, n);
    mat_print("\n(i) A + B =", c, n);
    printf("    [Time: O(n^2), Space: O(n^2)]\n");
    mat_free(c);

    Mat p2 = mat_mul_strassen(a, b, n);
    mat_print("\n(ii) A*B (Strassen) =", p2, n);
    printf("    [Time: O(n^2.81), Space: O(n^2)]\n");
    mat_free(p2);

    printf("\n(iii) is B a zero matrix? %s [Time: O(n^2)]\n", is_zero_matrix(b, n) ? "yes" : "no");

    printf("\n(iv) is A symmetric? %s [Time: O(n^2)]\n", is_symmetric_optimal(a, n) ? "yes" : "no");
    printf("     is B symmetric? %s\n", is_symmetric_optimal(b, n) ? "yes" : "no");

    printf("\n(v) det(A) [LU] = %.4f [Time: O(n^3)]\n", det_lu(a, n));

    Mat at = mat_copy(a, n);
    transpose_inplace(at, n);
    mat_print("\n(vi) transpose(A) in place =", at, n);
    printf("    [Time: O(n^2), Space: O(1)]\n");
    mat_free(at);

    double eigvec[4];
    double lam = power_iteration(a, n, eigvec, 200);
    printf("\n(vii) power iteration -> dominant eigenvalue ~ %.4f [Time: O(k*n^2)]\n", lam);
    printf("      eigenvector ~ [%.4f, %.4f, %.4f, %.4f]\n", eigvec[0], eigvec[1], eigvec[2], eigvec[3]);

    mat_free(a); mat_free(b);
}

static void bench(int n) {
    Mat a = mat_random(n, 1), b = mat_random(n, 2);
    double t0, t1;
    printf("%d", n);

    t0 = now_sec(); Mat c = mat_add(a, b, n); t1 = now_sec(); mat_free(c);
    printf(",%.9f", t1 - t0);

    t0 = now_sec(); Mat p2 = mat_mul_strassen(a, b, n); t1 = now_sec();
    printf(",%.9f", t1 - t0); mat_free(p2);
    
    Mat zeroMat = mat_alloc(n); 
    t0 = now_sec(); volatile int z = is_zero_matrix(zeroMat, n); (void)z; t1 = now_sec();
    printf(",%.9f", t1 - t0); mat_free(zeroMat);

    Mat symMat = mat_alloc(n);
    for (int i = 0; i < n; i++) for (int j = i; j < n; j++) {
        double v = (double)((i * 37 + j * 17) % 23) - 11;
        symMat[i*n+j] = v; symMat[j*n+i] = v;
    }
    t0 = now_sec(); volatile int s = is_symmetric_optimal(symMat, n); (void)s; t1 = now_sec();
    printf(",%.9f", t1 - t0); mat_free(symMat);

    Mat at = mat_copy(a, n);
    t0 = now_sec(); transpose_inplace(at, n); t1 = now_sec();
    printf(",%.9f", t1 - t0); mat_free(at);

    t0 = now_sec(); volatile double d2 = det_lu(a, n); (void)d2; t1 = now_sec();
    printf(",%.9f", t1 - t0);

    if (n <= 200) {
        double *eigvec = malloc(sizeof(double) * n);
        t0 = now_sec(); volatile double lam = power_iteration(a, n, eigvec, 50); (void)lam; t1 = now_sec();
        printf(",%.9f\n", t1 - t0); free(eigvec);
    } else {
        printf(",NaN\n");
    }

    mat_free(a); mat_free(b);
}

int main(int argc, char **argv) {
    if (argc >= 3 && strcmp(argv[1], "bench") == 0) bench(atoi(argv[2]));
    else demo();
    return 0;
}