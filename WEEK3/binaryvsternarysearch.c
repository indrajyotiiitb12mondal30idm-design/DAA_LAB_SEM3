#include <stdio.h>
#include <stdlib.h>

/* global counters - reset before every search call */
long long g_comparisons = 0;

/* ---------------------------------------------------------------------
   1a. BINARY SEARCH - Recursive version
   Every call does AT MOST 2 comparisons against the array:
        - one to test arr[mid] == key
        - one to test arr[mid] < key   (to decide which half to recurse on)
   --------------------------------------------------------------------- */
int binarySearchRecursive(int arr[], int low, int high, int key) {
    if (low > high) return -1;                 /* base case: not found   */

    int mid = low + (high - low) / 2;

    g_comparisons++;                            /* comparison #1 */
    if (arr[mid] == key) return mid;

    g_comparisons++;                            /* comparison #2 */
    if (arr[mid] < key)
        return binarySearchRecursive(arr, mid + 1, high, key);
    else
        return binarySearchRecursive(arr, low, mid - 1, key);
}

/* ---------------------------------------------------------------------
   1b. BINARY SEARCH - Iterative version (identical comparison pattern,
   but uses O(1) auxiliary space instead of O(log n) recursion stack)
   --------------------------------------------------------------------- */
int binarySearchIterative(int arr[], int n, int key) {
    int low = 0, high = n - 1;
    while (low <= high) {
        int mid = low + (high - low) / 2;

        g_comparisons++;
        if (arr[mid] == key) return mid;

        g_comparisons++;
        if (arr[mid] < key) low = mid + 1;
        else high = mid - 1;
    }
    return -1;
}

/* ---------------------------------------------------------------------
   2a. TERNARY SEARCH - Recursive version
   Every call splits [low,high] into 3 nearly equal parts using two
   internal points mid1, mid2.  In the worst case it performs 4
   comparisons per call:
        - arr[mid1] == key
        - arr[mid2] == key
        - key < arr[mid1]   (decide if answer is in the LEFT third)
        - key > arr[mid2]   (decide if answer is in the RIGHT third)
   --------------------------------------------------------------------- */
int ternarySearchRecursive(int arr[], int low, int high, int key) {
    if (low > high) return -1;

    int mid1 = low + (high - low) / 3;
    int mid2 = high - (high - low) / 3;

    g_comparisons++;
    if (arr[mid1] == key) return mid1;

    g_comparisons++;
    if (arr[mid2] == key) return mid2;

    g_comparisons++;
    if (key < arr[mid1])
        return ternarySearchRecursive(arr, low, mid1 - 1, key);

    g_comparisons++;
    if (key > arr[mid2])
        return ternarySearchRecursive(arr, mid2 + 1, high, key);

    return ternarySearchRecursive(arr, mid1 + 1, mid2 - 1, key);
}

/* ---------------------------------------------------------------------
   2b. TERNARY SEARCH - Iterative version
   --------------------------------------------------------------------- */
int ternarySearchIterative(int arr[], int n, int key) {
    int low = 0, high = n - 1;
    while (low <= high) {
        int mid1 = low + (high - low) / 3;
        int mid2 = high - (high - low) / 3;

        g_comparisons++;
        if (arr[mid1] == key) return mid1;

        g_comparisons++;
        if (arr[mid2] == key) return mid2;

        g_comparisons++;
        if (key < arr[mid1]) { high = mid1 - 1; continue; }

        g_comparisons++;
        if (key > arr[mid2]) { low = mid2 + 1; continue; }

        low = mid1 + 1; high = mid2 - 1;
    }
    return -1;
}

/* Utility: count recursion "levels" (depth) instead of comparisons,
   used only to build the illustrative depth-vs-n plot.                */
int binaryDepth(int n) {
    int levels = 0, size = n;
    while (size > 0) { levels++; size /= 2; }
    return levels;
}
int ternaryDepth(int n) {
    int levels = 0, size = n;
    while (size > 0) { levels++; size /= 3; }
    return levels;
}

/* ---------------------------------------------------------------------
   Experiment driver
   For every array size n in a chosen list, we build a sorted array
   0,1,2,...,n-1 and search for EVERY possible key from 0..n-1 as well
   as one guaranteed "not found" key, recording the MAXIMUM (worst
   case) number of comparisons seen for binary and ternary search.
   We also record recursion depth (levels) for the illustrative plot.
   Results are written to a CSV file for plotting.
   --------------------------------------------------------------------- */
int main(void) {
    int sizes[] = {4,8,16,32,64,128,256,512,1024,2048,4096,8192,16384,
                   32768,65536,131072,262144,524288,1048576};
    int numSizes = sizeof(sizes)/sizeof(sizes[0]);

    FILE *fp = fopen("q1_data.csv", "w");
    if (!fp) { perror("fopen"); return 1; }
    fprintf(fp, "n,binary_worst,ternary_worst,binary_levels,ternary_levels\n");

    for (int s = 0; s < numSizes; s++) {
        int n = sizes[s];
        int *arr = malloc(sizeof(int) * n);
        for (int i = 0; i < n; i++) arr[i] = i;   /* sorted array 0..n-1 */

        long long binWorst = 0, terWorst = 0;

        /* Search for every key 0..n-1 plus a missing key n (not present) */
        for (int key = 0; key <= n; key++) {
            g_comparisons = 0;
            binarySearchIterative(arr, n, key);
            if (g_comparisons > binWorst) binWorst = g_comparisons;

            g_comparisons = 0;
            ternarySearchIterative(arr, n, key);
            if (g_comparisons > terWorst) terWorst = g_comparisons;
        }

        fprintf(fp, "%d,%lld,%lld,%d,%d\n",
                n, binWorst, terWorst, binaryDepth(n), ternaryDepth(n));
        printf("n=%8d  binary_worst=%4lld  ternary_worst=%4lld  "
               "binary_levels=%2d  ternary_levels=%2d\n",
               n, binWorst, terWorst, binaryDepth(n), ternaryDepth(n));

        free(arr);
    }

    fclose(fp);
    printf("\nCSV written to q1_data.csv\n");
    return 0;
}