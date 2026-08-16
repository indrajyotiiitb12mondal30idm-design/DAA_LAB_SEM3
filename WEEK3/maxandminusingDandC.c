#include <stdio.h>
#include <stdlib.h>

long long g_comparisons = 0;

typedef struct { int mx, mn; } Pair;

/* ---------------------------------------------------------------------
   (1) NAIVE linear scan: 2 comparisons per element -> 2(n-1) total
   --------------------------------------------------------------------- */
Pair naiveMaxMin(int arr[], int n) {
    int mx = arr[0], mn = arr[0];
    for (int i = 1; i < n; i++) {
        g_comparisons++;
        if (arr[i] > mx) mx = arr[i];
        g_comparisons++;
        if (arr[i] < mn) mn = arr[i];
    }
    Pair p = {mx, mn};
    return p;
}

/* ---------------------------------------------------------------------
   (2) DIVIDE & CONQUER (recursive), the "simple" solution.
   T(n) = 2T(n/2) + 2  =>  T(n) ~ 3n/2 - 2
   --------------------------------------------------------------------- */
Pair maxMinDC(int arr[], int low, int high) {
    int n = high - low + 1;
    Pair result;

    if (n == 1) {                       /* base case: single element, 0 cmp */
        result.mx = result.mn = arr[low];
        return result;
    }
    if (n == 2) {                       /* base case: pair, 1 comparison */
        g_comparisons++;
        if (arr[low] > arr[high]) {
            result.mx = arr[low];  result.mn = arr[high];
        } else {
            result.mx = arr[high]; result.mn = arr[low];
        }
        return result;
    }

    int mid = low + (high - low) / 2;
    Pair left  = maxMinDC(arr, low, mid);
    Pair right = maxMinDC(arr, mid + 1, high);

    g_comparisons++;
    result.mx = (left.mx > right.mx) ? left.mx : right.mx;
    g_comparisons++;
    result.mn = (left.mn < right.mn) ? left.mn : right.mn;
    return result;
}

/* ---------------------------------------------------------------------
   (3) ITERATIVE TOURNAMENT PAIRING, the "best" solution.
   Exactly the same 3n/2-ish comparison count as (2), but O(1) space:
   we never recurse, we just sweep the array two elements at a time.
   --------------------------------------------------------------------- */
Pair maxMinIterative(int arr[], int n) {
    Pair result;
    int i = 0;

    if (n % 2 == 0) {
        /* initialise using the FIRST pair - only 1 comparison, no
           wasted comparisons against a "seed" value                  */
        g_comparisons++;
        if (arr[0] > arr[1]) { result.mx = arr[0]; result.mn = arr[1]; }
        else                 { result.mx = arr[1]; result.mn = arr[0]; }
        i = 2;
    } else {
        /* odd n: seed with the first element alone (0 comparisons),
           then process the rest in pairs                             */
        result.mx = result.mn = arr[0];
        i = 1;
    }

    while (i < n) {
        int a = arr[i], b = arr[i + 1];
        int localMax, localMin;

        g_comparisons++;                 /* 1: compare pair internally */
        if (a > b) { localMax = a; localMin = b; }
        else       { localMax = b; localMin = a; }

        g_comparisons++;                 /* 2: localMax vs running max */
        if (localMax > result.mx) result.mx = localMax;

        g_comparisons++;                 /* 3: localMin vs running min */
        if (localMin < result.mn) result.mn = localMin;

        i += 2;
    }
    return result;
}

/* ---------------------------------------------------------------------
   Correctness check + comparison-count harness
   --------------------------------------------------------------------- */
void shuffle(int *a, int n) {
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int t = a[i]; a[i] = a[j]; a[j] = t;
    }
}

int main(void) {
    srand(42);
    int fails = 0;

    /* correctness across many random arrays, sizes 1..500 */
    for (int n = 1; n <= 500; n++) {
        int *arr = malloc(sizeof(int) * n);
        for (int i = 0; i < n; i++) arr[i] = i;
        shuffle(arr, n);

        Pair p1 = naiveMaxMin(arr, n);
        Pair p2 = maxMinDC(arr, 0, n - 1);
        Pair p3 = maxMinIterative(arr, n);

        if (p1.mx != p2.mx || p1.mn != p2.mn || p2.mx != p3.mx || p2.mn != p3.mn) {
            printf("MISMATCH at n=%d\n", n);
            fails++;
        }
        free(arr);
    }
    printf("Correctness check across n=1..500: %d failures\n\n", fails);

    /* comparison counts for plotting / validating the 3n/2 bound */
    FILE *fp = fopen("q3_data.csv", "w");
    if (!fp) { perror("fopen"); return 1; }
    fprintf(fp, "n,naive,dc_recursive,iterative,bound_3n_2\n");

    int sizes[] = {2,4,8,10,16,20,32,50,64,100,128,200,256,500,512,1000,
                   1024,2000,2048,5000,8192,10000,16384,32768,50000,65536,
                   100000,200000};
    for (unsigned s = 0; s < sizeof(sizes)/sizeof(sizes[0]); s++) {
        int n = sizes[s];
        int *arr = malloc(sizeof(int) * n);
        for (int i = 0; i < n; i++) arr[i] = i;
        shuffle(arr, n);

        g_comparisons = 0; naiveMaxMin(arr, n);          long long cNaive = g_comparisons;
        g_comparisons = 0; maxMinDC(arr, 0, n - 1);       long long cDC    = g_comparisons;
        g_comparisons = 0; maxMinIterative(arr, n);       long long cIter  = g_comparisons;

        double bound = (n % 2 == 0) ? (3.0 * n / 2.0 - 2) : (3.0 * (n - 1) / 2.0);

        fprintf(fp, "%d,%lld,%lld,%lld,%.1f\n", n, cNaive, cDC, cIter, bound);
        printf("n=%7d  naive=%7lld  DC(recursive)=%7lld  iterative=%7lld  bound(3n/2-ish)=%.1f\n",
               n, cNaive, cDC, cIter, bound);
        free(arr);
    }
    fclose(fp);
    return 0;
}