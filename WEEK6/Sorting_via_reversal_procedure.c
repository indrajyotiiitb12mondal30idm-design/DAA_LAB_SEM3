#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

static long g_reversalCount;
static long long g_totalCost;

static void reverse(int *p, int i, int j) {
    g_reversalCount++;
    g_totalCost += (j - i + 1);
    while (i < j) { int t = p[i]; p[i] = p[j]; p[j] = t; i++; j--; }
}

/* SORT WITH O(n) REVERSALS */
void sort_by_reversal_countOptimal(int *p, int n) {
    for (int i = 0; i < n - 1; i++) {
        int target = i + 1;             
        int j = i;
        while (p[j] != target) j++;     
        if (j != i) reverse(p, i, j);
    }
}

/* SORT WITH O(n log^2 n) TOTAL COST */
static void rotate_blocks(int *p, int a, int b, int c) {
    if (a > b || b + 1 > c) return;
    reverse(p, a, b);
    reverse(p, b + 1, c);
    reverse(p, a, c);
}

static int upper_bound(const int *p, int lo, int hi, int key) {
    int orig_hi = hi;
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        if (p[mid] > key) hi = mid - 1; else lo = mid + 1;
    }
    return lo > orig_hi ? orig_hi + 1 : lo;
}

static void merge_by_reversal(int *p, int l, int m, int r) {
    if (l > m || m + 1 > r) return;         
    if (p[m] <= p[m + 1]) return;          
    int n1 = m - l + 1, n2 = r - m;

    if (n1 <= n2) {
        int qmid = m + 1 + (n2 - 1) / 2;          
        int idx = upper_bound(p, l, m, p[qmid]);   
        rotate_blocks(p, idx, m, qmid);
        int newBoundary = idx + (qmid - m) - 1;    
        merge_by_reversal(p, l, idx - 1, newBoundary);
        merge_by_reversal(p, newBoundary + 1, qmid, r);
    } else {
        int pmid = l + (n1 - 1) / 2;                
        int idx = upper_bound(p, m + 1, r, p[pmid]); 
        rotate_blocks(p, pmid + 1, m, idx - 1);
        int newBoundary = pmid + (idx - 1 - m);
        merge_by_reversal(p, l, pmid, newBoundary);
        merge_by_reversal(p, newBoundary + 1, idx - 1, r);
    }
}

static void merge_sort_by_reversal(int *p, int l, int r) {
    if (l >= r) return;
    int m = l + (r - l) / 2;
    merge_sort_by_reversal(p, l, m);
    merge_sort_by_reversal(p, m + 1, r);
    merge_by_reversal(p, l, m, r);
}

static int is_sorted(const int *p, int n) {
    for (int i = 1; i < n; i++) if (p[i - 1] > p[i]) return 0;
    return 1;
}
static void shuffle(int *p, int n, unsigned seed) {
    srand(seed);
    for (int i = 0; i < n; i++) p[i] = i + 1;
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int t = p[i]; p[i] = p[j]; p[j] = t;
    }
}

static void print_arr(const char *label, const int *a, int n) {
    printf("%s [", label);
    for (int i = 0; i < n; i++) printf("%d%s", a[i], i + 1 < n ? ", " : "");
    printf("]\n");
}

static void demo(void) {
    int n = 12;
    int p[12]; shuffle(p, n, 99);
    
    printf("\n--- Mathematical Proof Validation ---\n");
    print_arr("Random permutation:", p, n);

    int *pa = malloc(sizeof(int) * n); memcpy(pa, p, sizeof(int) * n);
    g_reversalCount = 0; g_totalCost = 0;
    sort_by_reversal_countOptimal(pa, n);
    print_arr("\n[Bullet 1] sorted (selection-by-reversal):", pa, n);
    printf("         reversals used = %ld  (Max allowed: n-1 = %d)\n", g_reversalCount, n - 1);
    printf("         total cost = %lld\n", g_totalCost);
    printf("         is_sorted = %s\n", is_sorted(pa, n) ? "yes" : "NO");
    printf("         Proof: Iteratively reversing subarray from current position i to the target element's location mathematically guarantees a sorted array in at most n-1 reversals.\n");
    free(pa);

    int *pb = malloc(sizeof(int) * n); memcpy(pb, p, sizeof(int) * n);
    g_reversalCount = 0; g_totalCost = 0;
    merge_sort_by_reversal(pb, 0, n - 1);
    print_arr("\n[Bullet 2] sorted (merge-sort-by-reversal):", pb, n);
    printf("         reversals used = %ld\n", g_reversalCount);
    printf("         total cost (sum of lengths) = %lld   (target: O(n log^2 n))\n", g_totalCost);
    printf("         is_sorted = %s\n", is_sorted(pb, n) ? "yes" : "NO");
    printf("         Proof: 3-reversal block rotations cost O(k). In-place merge recurrence M(k) = 2M(k/2) + O(k) = O(k log k). Overall sort recurrence T(n) = 2T(n/2) + O(n log n) yields O(n log^2 n).\n");
    free(pb);
}

static void bench(int n) {
    int *p = malloc(sizeof(int) * n);

    shuffle(p, n, 123);
    g_reversalCount = 0; g_totalCost = 0;
    sort_by_reversal_countOptimal(p, n);
    if (!is_sorted(p, n)) { fprintf(stderr, "BUG: Part A failed to sort n=%d\n", n); exit(1); }
    long reversalsA = g_reversalCount; long long costA = g_totalCost;

    shuffle(p, n, 123); 
    g_reversalCount = 0; g_totalCost = 0;
    merge_sort_by_reversal(p, 0, n - 1);
    if (!is_sorted(p, n)) { fprintf(stderr, "BUG: Part B failed to sort n=%d\n", n); exit(1); }
    long reversalsB = g_reversalCount; long long costB = g_totalCost;

    printf("%d,%ld,%lld,%ld,%lld\n", n, reversalsA, costA, reversalsB, costB);
    free(p);
}

int main(int argc, char **argv) {
    if (argc >= 3 && strcmp(argv[1], "bench") == 0) bench(atoi(argv[2]));
    else demo();
    return 0;
}


/*
Time Complexity (Total Cost): O(n²)

Total Reversals: O(n) (strictly maximum n-1)

Space Complexity: O(1) auxiliary
*/