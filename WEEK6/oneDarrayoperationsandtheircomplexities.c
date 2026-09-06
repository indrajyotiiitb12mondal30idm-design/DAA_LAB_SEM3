#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#if defined(_WIN32)
#include <windows.h>
#else
#define _POSIX_C_SOURCE 199309L
#endif

/* ---------- small utility: high resolution timer -------------------------*/
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

static int* random_array(int n, int seed) {
    srand(seed);
    int *a = malloc(sizeof(int) * n);
    for (int i = 0; i < n; i++) a[i] = rand() % (n * 5 + 1); /* allow repeats -> mode is meaningful */
    return a;
}

static int* copy_array(const int *a, int n) {
    int *b = malloc(sizeof(int) * n);
    memcpy(b, a, sizeof(int) * n);
    return b;
}

/* (i) Finding the maximum element */

/*
Time complexity: O(n)
Space complexity: O(1) auxiliary
*/
int find_max(const int *a, int n) {
    int m = a[0];
    for (int i = 1; i < n; i++)
        if (a[i] > m) m = a[i];
    return m;
}

/* (ii) First and second largest */

/*
Time complexity: O(n)
Space complexity: O(1) auxiliary 
*/
typedef struct { int first, second; long comparisons; } Top2;

Top2 top2_naive(const int *a, int n) {
    Top2 r; long cmp = 0;
    r.first = a[0] > a[1] ? a[0] : a[1];
    r.second = a[0] > a[1] ? a[1] : a[0];
    cmp++; 
    for (int i = 2; i < n; i++) {
        cmp++;
        if (a[i] > r.first) {
            cmp++;
            r.second = r.first;
            r.first = a[i];
        } else {
            cmp++;
            if (a[i] > r.second) r.second = a[i];
        }
    }
    r.comparisons = cmp;
    return r;
}

/* (iii) Mean */

/*
Time complexity: O(n)
Space complexity: O(1) auxiliary 
*/
double find_mean(const int *a, int n) {
    long long sum = 0;
    for (int i = 0; i < n; i++) sum += a[i];
    return (double) sum / n;
}

/* (iv) Median */

/*
Time complexity: O(n log n) (due to sorting)
Space complexity: O(n) auxiliary 
*/
static int cmp_int(const void *x, const void *y) { return (*(int*)x - *(int*)y); }

double median_sort(const int *a, int n) {
    int *b = copy_array(a, n);
    qsort(b, n, sizeof(int), cmp_int);
    double m = (n % 2) ? b[n/2] : (b[n/2 - 1] + b[n/2]) / 2.0;
    free(b);
    return m;
}

/* (v) Standard deviation */

/*
Time complexity: O(n)
Space complexity: O(1) auxiliary 
*/
double stddev_twopass(const int *a, int n) {
    double mean = find_mean(a, n);
    double ss = 0;
    for (int i = 0; i < n; i++) { double d = a[i] - mean; ss += d * d; }
    return sqrt(ss / n);
}

/* (vi) Mode */

/*
Time complexity: O(n^2)
Space complexity: O(1) auxiliary 
*/
int mode_naive(const int *a, int n) {
    int bestVal = a[0], bestCount = 0;
    for (int i = 0; i < n; i++) {
        int c = 0;
        for (int j = 0; j < n; j++) if (a[j] == a[i]) c++;
        if (c > bestCount) { bestCount = c; bestVal = a[i]; }
    }
    return bestVal;
}

/* (vii) Removing all duplicates */
/*
Time complexity: O(n^2)
Space complexity: O(n) auxiliary 
*/
int dedup_naive(const int *a, int n, int *out) {
    int k = 0;
    for (int i = 0; i < n; i++) {
        int seen = 0;
        for (int j = 0; j < k; j++) if (out[j] == a[i]) { seen = 1; break; }
        if (!seen) out[k++] = a[i];
    }
    return k;
}

/* (viii) Reversing the array */

/*
Time complexity: O(n)
Space complexity: O(1) auxiliary 
*/
void reverse_array(int *a, int n) {
    int i = 0, j = n - 1;
    while (i < j) { int t = a[i]; a[i] = a[j]; a[j] = t; i++; j--; }
}

/* (ix) Partition w.r.t. a random pivot */

/*
Time complexity: O(n)
Space complexity: O(n) auxiliary 
*/
int partition_extra_space(const int *a, int n, int pivot, int *out) {
    int *ge = malloc(sizeof(int) * n), *lt = malloc(sizeof(int) * n);
    int gk = 0, lk = 0;
    for (int i = 0; i < n; i++) { if (a[i] >= pivot) ge[gk++] = a[i]; else lt[lk++] = a[i]; }
    memcpy(out, ge, sizeof(int) * gk);
    memcpy(out + gk, lt, sizeof(int) * lk);
    free(ge); free(lt);
    return gk; 
}

static void print_arr(const char *label, const int *a, int n) {
    printf("%s [", label);
    for (int i = 0; i < n; i++) printf("%d%s", a[i], i + 1 < n ? ", " : "");
    printf("]\n");
}

static void demo(void) {
    int a[] = {7, 2, 9, 4, 4, 1, 3, 5, 2, 4};
    int n = sizeof(a) / sizeof(a[0]);
    print_arr("Input array:", a, n);
    printf("\n--- Complexity Analysis Validation ---\n");

    printf("\n(i) max = %d [Time: O(n), Space: O(1)]\n", find_max(a, n));
Top2 tn = top2_naive(a, n);
    printf("\n(ii) naive:      first=%d second=%d  comparisons=%ld [Time: O(n), Space: O(1)]\n", tn.first, tn.second, tn.comparisons);

    printf("\n(iii) mean = %.4f [Time: O(n), Space: O(1)]\n", find_mean(a, n));

    printf("\n(iv) median (sort)       = %.2f [Time: O(n log n), Space: O(n)]\n", median_sort(a, n));

    printf("\n(v) stddev (two-pass) = %.6f [Time: O(n), Space: O(1)]\n", stddev_twopass(a, n));

    printf("\n(vi) mode (naive) = %d [Time: O(n^2), Space: O(1)]\n", mode_naive(a, n));

    int out[10];
    int k1 = dedup_naive(a, n, out);
    print_arr("\n(vii) dedup (naive):", out, k1);
    printf("      [Time: O(n^2), Space: O(n)]\n");

    int b[10]; memcpy(b, a, sizeof(a));
    reverse_array(b, n);
    print_arr("\n(viii) reversed:", b, n);
    printf("       [Time: O(n), Space: O(1)]\n");

    int pivot = 5;
    int out2[10];
    int bnd1 = partition_extra_space(a, n, pivot, out2);
    print_arr("\n(ix) partition (extra space):", out2, n);
    printf("      boundary index (first %d elements are >= %d) = %d\n", bnd1, pivot, bnd1);
    printf("      [Time: O(n), Space: O(n)]\n");
}

static void bench(int n) {
    int *a = random_array(n, 42);
    double t0, t1;
    printf("%d", n);

    t0 = now_sec(); volatile int mx = find_max(a, n); (void)mx; t1 = now_sec();
    printf(",%.9f", t1 - t0);

    t0 = now_sec(); Top2 tn = top2_naive(a, n); t1 = now_sec();
    printf(",%.9f", t1 - t0);
    printf(",%ld", tn.comparisons);

    t0 = now_sec(); volatile double mean = find_mean(a, n); (void)mean; t1 = now_sec();
    printf(",%.9f", t1 - t0);

    t0 = now_sec(); volatile double med1 = median_sort(a, n); (void)med1; t1 = now_sec();
    printf(",%.9f", t1 - t0);

    t0 = now_sec(); volatile double s1 = stddev_twopass(a, n); (void)s1; t1 = now_sec();
    printf(",%.9f", t1 - t0);

    if (n <= 4000) {
        t0 = now_sec(); volatile int md1 = mode_naive(a, n); (void)md1; t1 = now_sec();
        printf(",%.9f", t1 - t0);
    } else printf(",NaN");

    int *out = malloc(sizeof(int) * n);
    if (n <= 4000) {
        t0 = now_sec(); dedup_naive(a, n, out); t1 = now_sec();
        printf(",%.9f", t1 - t0);
    } else printf(",NaN");
    free(out);

    int *b = copy_array(a, n);
    t0 = now_sec(); reverse_array(b, n); t1 = now_sec();
    printf(",%.9f", t1 - t0);
    free(b);

    int pivot = a[n/2];
    int *out2 = malloc(sizeof(int) * n);
    t0 = now_sec(); partition_extra_space(a, n, pivot, out2); t1 = now_sec();
    printf(",%.9f\n", t1 - t0);
    free(out2);

    free(a);
}

int main(int argc, char **argv) {
    if (argc >= 3 && strcmp(argv[1], "bench") == 0) {
        bench(atoi(argv[2]));
    } else {
        demo();
    }
    return 0;
}