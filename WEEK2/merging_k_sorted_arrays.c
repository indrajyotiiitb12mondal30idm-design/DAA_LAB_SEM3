#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#ifdef _WIN32
#include <windows.h>
#endif

static double now_ns(void) {
#ifdef _WIN32
    static LARGE_INTEGER freq = {0};
    if (freq.QuadPart == 0) {
        if (!QueryPerformanceFrequency(&freq))
            return (double)clock() / (double)CLOCKS_PER_SEC * 1e9;
    }
    LARGE_INTEGER t;
    QueryPerformanceCounter(&t);
    return (double)t.QuadPart * 1e9 / (double)freq.QuadPart;
#else
    return (double)clock() / (double)CLOCKS_PER_SEC * 1e9;
#endif
}

/* plain 2-way merge of a[l..m] and a[m+1..r] using scratch buffer tmp */
static void merge2(int *a, int *tmp, int l, int m, int r) {
    int i = l, j = m + 1, w = l;
    while (i <= m && j <= r) tmp[w++] = (a[i] <= a[j]) ? a[i++] : a[j++];
    while (i <= m) tmp[w++] = a[i++];
    while (j <= r) tmp[w++] = a[j++];
    for (int x = l; x <= r; x++) a[x] = tmp[x];
}

/* =====================================================================
   METHOD 1 : SEQUENTIAL MERGE
   arrs[0..k-1] each holds n already-sorted integers.
   Returns a freshly malloc'ed array of length k*n.
   ===================================================================== */
static int* merge_sequential(int **arrs, int k, int n) {
    int cap = n;                                   /* size of the running result */
    int *result = malloc(sizeof(int) * n);
    memcpy(result, arrs[0], sizeof(int) * n);
    int *tmp = malloc(sizeof(int) * (size_t)k * n);

    for (int idx = 1; idx < k; idx++) {
        int newcap = cap + n;
        int *combined = malloc(sizeof(int) * newcap);
        memcpy(combined, result, sizeof(int) * cap);
        memcpy(combined + cap, arrs[idx], sizeof(int) * n);
        merge2(combined, tmp, 0, cap - 1, newcap - 1);
        free(result);
        result = combined;
        cap = newcap;
    }
    free(tmp);
    return result;
}

/* =====================================================================
   METHOD 2 : PAIRWISE (DIVIDE & CONQUER) MERGE
   ===================================================================== */
static int* merge_pairwise(int **arrs, int k, int n) {
    /* "current" is an array of k pointers, each to a sorted block;
       "lens" holds the length of each of those blocks (all = n at first) */
    int **current = malloc(sizeof(int*) * k);
    int  *lens    = malloc(sizeof(int)  * k);
    for (int i = 0; i < k; i++) {
        current[i] = malloc(sizeof(int) * n);
        memcpy(current[i], arrs[i], sizeof(int) * n);
        lens[i] = n;
    }
    int count = k;

    while (count > 1) {
        int newCount = (count + 1) / 2;
        int **next = malloc(sizeof(int*) * newCount);
        int  *nextLens = malloc(sizeof(int) * newCount);

        for (int i = 0; i < newCount; i++) {
            int a = 2 * i, b = 2 * i + 1;
            if (b >= count) {                       /* odd one out: carry forward */
                next[i] = current[a];
                nextLens[i] = lens[a];
            } else {
                int totalLen = lens[a] + lens[b];
                int *merged = malloc(sizeof(int) * totalLen);
                int *tmp    = malloc(sizeof(int) * totalLen);
                memcpy(merged, current[a], sizeof(int) * lens[a]);
                memcpy(merged + lens[a], current[b], sizeof(int) * lens[b]);
                merge2(merged, tmp, 0, lens[a] - 1, totalLen - 1);
                free(tmp);
                free(current[a]); free(current[b]);
                next[i] = merged;
                nextLens[i] = totalLen;
            }
        }
        free(current); free(lens);
        current = next; lens = nextLens; count = newCount;
    }
    int *result = current[0];
    free(current); free(lens);
    return result;
}

/* =====================================================================
   METHOD 3 (BONUS) : k-WAY MERGE WITH A BINARY MIN-HEAP
   heap entries store: value, which source array, index within that array
   ===================================================================== */
typedef struct { int value, arr, idx; } HeapEntry;

static void heap_push(HeapEntry *h, int *size, HeapEntry e) {
    int i = (*size)++;
    h[i] = e;
    while (i > 0) {
        int p = (i - 1) / 2;
        if (h[p].value <= h[i].value) break;
        HeapEntry t = h[p]; h[p] = h[i]; h[i] = t;
        i = p;
    }
}
static HeapEntry heap_pop(HeapEntry *h, int *size) {
    HeapEntry top = h[0];
    h[0] = h[--(*size)];
    int i = 0;
    for (;;) {
        int l = 2 * i + 1, r = 2 * i + 2, smallest = i;
        if (l < *size && h[l].value < h[smallest].value) smallest = l;
        if (r < *size && h[r].value < h[smallest].value) smallest = r;
        if (smallest == i) break;
        HeapEntry t = h[i]; h[i] = h[smallest]; h[smallest] = t;
        i = smallest;
    }
    return top;
}
static int* merge_heap(int **arrs, int k, int n) {
    int *result = malloc(sizeof(int) * (size_t)k * n);
    HeapEntry *heap = malloc(sizeof(HeapEntry) * k);   /* only O(k) extra space! */
    int hsize = 0;
    for (int i = 0; i < k; i++)
        heap_push(heap, &hsize, (HeapEntry){arrs[i][0], i, 0});

    int w = 0;
    while (hsize > 0) {
        HeapEntry top = heap_pop(heap, &hsize);
        result[w++] = top.value;
        int nextIdx = top.idx + 1;
        if (nextIdx < n)
            heap_push(heap, &hsize, (HeapEntry){arrs[top.arr][nextIdx], top.arr, nextIdx});
    }
    free(heap);
    return result;
}

static int is_sorted(int *a, int len) {
    for (int i = 1; i < len; i++) if (a[i - 1] > a[i]) return 0;
    return 1;
}
static int cmp_int(const void *a, const void *b) { return (*(int*)a - *(int*)b); }

static int** make_sorted_arrays(int k, int n) {
    int **arrs = malloc(sizeof(int*) * k);
    for (int i = 0; i < k; i++) {
        arrs[i] = malloc(sizeof(int) * n);
        for (int j = 0; j < n; j++) arrs[i][j] = rand();
        qsort(arrs[i], n, sizeof(int), cmp_int);
    }
    return arrs;
}
static void free_arrays(int **arrs, int k) {
    for (int i = 0; i < k; i++) free(arrs[i]);
    free(arrs);
}

#define TRIALS 5

/* runs all 3 methods once for the given (k, n) and appends one CSV
   line per method to fp; "sweep" is just a text label ("vary_n"/"vary_k") */
static void run_experiment(FILE *fp, int k, int n) {
    int **arrs = make_sorted_arrays(k, n);

    double best1 = 1e18, best2 = 1e18, best3 = 1e18;
    for (int t = 0; t < TRIALS; t++) {
        double t0 = now_ns();
        int *r1 = merge_sequential(arrs, k, n);
        double dt = now_ns() - t0;
        if (!is_sorted(r1, k * n)) { fprintf(stderr, "BUG in Method1\n"); exit(1); }
        if (dt < best1) best1 = dt;
        free(r1);

        t0 = now_ns();
        int *r2 = merge_pairwise(arrs, k, n);
        dt = now_ns() - t0;
        if (!is_sorted(r2, k * n)) { fprintf(stderr, "BUG in Method2\n"); exit(1); }
        if (dt < best2) best2 = dt;
        free(r2);

        t0 = now_ns();
        int *r3 = merge_heap(arrs, k, n);
        dt = now_ns() - t0;
        if (!is_sorted(r3, k * n)) { fprintf(stderr, "BUG in Method3\n"); exit(1); }
        if (dt < best3) best3 = dt;
        free(r3);
    }
    long long best1_ns = llround(best1);
    long long best2_ns = llround(best2);
    long long best3_ns = llround(best3);
    fprintf(fp, "%d,%d,Method1_Sequential,%lld\n", n, k, best1_ns);
    fprintf(fp, "%d,%d,Method2_Pairwise,%lld\n",    n, k, best2_ns);
    fprintf(fp, "%d,%d,Method3_Heap,%lld\n",         n, k, best3_ns);
    printf("n=%6d k=%5d   seq=%10lld ns   pairwise=%10lld ns   heap=%10lld ns\n",
           n, k, best1_ns, best2_ns, best3_ns);

    free_arrays(arrs, k);
}

int main(void) {
    srand(11);

    /* ---------- Sweep A: fix k, grow n ---------- */
    FILE *fpA = fopen("q3_vary_n.csv", "w");
    if (!fpA) { perror("fopen q3_vary_n.csv"); return 1; }
    fprintf(fpA, "n,k,method,time_ns\n");
    int kFixed = 16;
    int nsA[] = {50, 100, 200, 400, 800, 1600, 3200, 6400, 12800, 25600};
    for (size_t i = 0; i < sizeof(nsA)/sizeof(nsA[0]); i++)
        run_experiment(fpA, kFixed, nsA[i]);
    fclose(fpA);

    /* ---------- Sweep B: fix n, grow k ---------- */
    FILE *fpB = fopen("q3_vary_k.csv", "w");
    if (!fpB) { perror("fopen q3_vary_k.csv"); fclose(fpA); return 1; }
    fprintf(fpB, "n,k,method,time_ns\n");
    int nFixed = 200;
    int ksB[] = {2, 4, 8, 16, 32, 64, 128, 256, 512, 1024};
    for (size_t i = 0; i < sizeof(ksB)/sizeof(ksB[0]); i++)
        run_experiment(fpB, ksB[i], nFixed);
    fclose(fpB);

    printf("Results written to q3_vary_n.csv and q3_vary_k.csv\n");
    return 0;
}