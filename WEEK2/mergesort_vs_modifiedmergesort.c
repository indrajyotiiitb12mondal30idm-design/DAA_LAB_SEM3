#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#endif

static long long g_comparisons = 0;   

static double now_ns(void) {
#ifdef _WIN32
    static LARGE_INTEGER freq = {0};
    if (freq.QuadPart == 0) QueryPerformanceFrequency(&freq);
    LARGE_INTEGER count;
    QueryPerformanceCounter(&count);
    return (double)count.QuadPart * 1e9 / (double)freq.QuadPart;
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1e9 + (double)ts.tv_nsec;
#endif
}

/* =====================================================================
   PART A : CLASSIC (2-WAY) MERGE SORT
   ===================================================================== */

/* merge():  merges two already-sorted runs a[l..m] and a[m+1..r]
             into one sorted run a[l..r], using a temporary buffer. */
static void merge(int *a, int *tmp, int l, int m, int r) {
    int i = l, j = m + 1, k = l;
    while (i <= m && j <= r) {
        g_comparisons++;
        if (a[i] <= a[j]) tmp[k++] = a[i++];
        else               tmp[k++] = a[j++];
    }
    while (i <= m) tmp[k++] = a[i++];
    while (j <= r) tmp[k++] = a[j++];
    for (int x = l; x <= r; x++) a[x] = tmp[x];
}

static void merge_sort(int *a, int *tmp, int l, int r) {
    if (l >= r) return;                 /* 0 or 1 element -> already sorted */
    int m = l + (r - l) / 2;            /* split into HALVES               */
    merge_sort(a, tmp, l, m);
    merge_sort(a, tmp, m + 1, r);
    merge(a, tmp, l, m, r);
}

/* =====================================================================
   PART B : MODIFIED (3-WAY) MERGE SORT
   ===================================================================== */

/* merge3():  merges three already-sorted runs
              a[l..m1], a[m1+1..m2], a[m2+1..r]  into one sorted run,
              by repeatedly picking the smallest of (up to) three
              current "front" elements.                                  */
static void merge3(int *a, int *tmp, int l, int m1, int m2, int r) {
    int i = l, j = m1 + 1, k = m2 + 1, w = l;
    while (i <= m1 && j <= m2 && k <= r) {
        g_comparisons += 2;                 /* 2 comparisons to find min of 3 */
        if (a[i] <= a[j] && a[i] <= a[k])      tmp[w++] = a[i++];
        else if (a[j] <= a[i] && a[j] <= a[k]) tmp[w++] = a[j++];
        else                                    tmp[w++] = a[k++];
    }
    /* at most one run is left now -> finish with ordinary 2-way merges */
    while (i <= m1 && j <= m2) { g_comparisons++;
        if (a[i] <= a[j]) tmp[w++] = a[i++]; else tmp[w++] = a[j++]; }
    while (j <= m2 && k <= r) { g_comparisons++;
        if (a[j] <= a[k]) tmp[w++] = a[j++]; else tmp[w++] = a[k++]; }
    while (i <= m1 && k <= r) { g_comparisons++;
        if (a[i] <= a[k]) tmp[w++] = a[i++]; else tmp[w++] = a[k++]; }
    while (i <= m1) tmp[w++] = a[i++];
    while (j <= m2) tmp[w++] = a[j++];
    while (k <= r)  tmp[w++] = a[k++];
    for (int x = l; x <= r; x++) a[x] = tmp[x];
}

static void merge_sort3(int *a, int *tmp, int l, int r) {
    int len = r - l + 1;
    if (len <= 1) return;
    if (len == 2) {                     /* base case: 2 elements, no true "thirds" */
        g_comparisons++;
        if (a[l] > a[r]) { int t = a[l]; a[l] = a[r]; a[r] = t; }
        return;
    }
    int third = len / 3;
    int m1 = l + third - 1;
    int m2 = l + 2 * third - 1;
    merge_sort3(a, tmp, l, m1);
    merge_sort3(a, tmp, m1 + 1, m2);
    merge_sort3(a, tmp, m2 + 1, r);
    merge3(a, tmp, l, m1, m2, r);
}

static int is_sorted(int *a, int n) {
    for (int i = 1; i < n; i++) if (a[i - 1] > a[i]) return 0;
    return 1;
}
static void fill_random(int *a, int n) {
    for (int i = 0; i < n; i++) a[i] = rand();
}

#define TRIALS 7   /* repeat each sort several times; keep the BEST (minimum)
                       time -- this is the standard way to cancel out one-off
                       noise from OS scheduling, interrupts, cache warm-up,
                       background load in a shared machine, etc.            */

int main(void) {
    srand(7);

    int sizes[] = {1000, 2000, 4000, 8000, 16000, 32000, 64000,
                    128000, 256000, 512000, 1000000};
    int num_sizes = (int)(sizeof(sizes) / sizeof(sizes[0]));

    FILE *fp = fopen("q2_timings.csv", "w");
    fprintf(fp, "n,algorithm,time_ns,comparisons\n");

    for (int s = 0; s < num_sizes; s++) {
        int n = sizes[s];
        int *master = malloc(sizeof(int) * n);
        int *work    = malloc(sizeof(int) * n);
        int *tmp     = malloc(sizeof(int) * n);
        fill_random(master, n);

        /* ---- classic merge sort : best-of-TRIALS timing ---- */
        double best2 = 1e18; long long cmp2 = 0;
        for (int trial = 0; trial < TRIALS; trial++) {
            memcpy(work, master, sizeof(int) * n);
            g_comparisons = 0;
            double t0 = now_ns();
            merge_sort(work, tmp, 0, n - 1);
            double t1 = now_ns();
            if (!is_sorted(work, n)) { fprintf(stderr, "BUG: merge_sort did not sort!\n"); return 1; }
            if (t1 - t0 < best2) { best2 = t1 - t0; cmp2 = g_comparisons; }
        }
        fprintf(fp, "%d,MergeSort2Way,%.0f,%lld\n", n, best2, cmp2);

        /* ---- modified 3-way merge sort : best-of-TRIALS timing ---- */
        double best3 = 1e18; long long cmp3 = 0;
        for (int trial = 0; trial < TRIALS; trial++) {
            memcpy(work, master, sizeof(int) * n);
            g_comparisons = 0;
            double t0 = now_ns();
            merge_sort3(work, tmp, 0, n - 1);
            double t1 = now_ns();
            if (!is_sorted(work, n)) { fprintf(stderr, "BUG: merge_sort3 did not sort!\n"); return 1; }
            if (t1 - t0 < best3) { best3 = t1 - t0; cmp3 = g_comparisons; }
        }
        fprintf(fp, "%d,MergeSort3Way,%.0f,%lld\n", n, best3, cmp3);

        printf("n=%8d  done  (2-way best=%.0f ns, 3-way best=%.0f ns)\n", n, best2, best3);
        free(master); free(work); free(tmp);
    }

    fclose(fp);
    printf("Results written to q2_timings.csv\n");
    return 0;
}
