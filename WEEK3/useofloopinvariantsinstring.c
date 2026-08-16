#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

long long g_cmp = 0, g_swap = 0;

void swapInt(int *a, int *b) { int t=*a; *a=*b; *b=t; g_swap++; }

/* ---------------------------------------------------------------------
   PART A: SELECTION SORT
   Matches the pseudocode: for i=1..n-1: find index of min in A[i..n],
   swap it into A[i].  (0-indexed here: i = 0..n-2)
   --------------------------------------------------------------------- */
void selectionSort(int A[], int n) {
    for (int i = 0; i < n - 1; i++) {
        int minIdx = i;
        for (int j = i + 1; j < n; j++) {
            g_cmp++;
            if (A[j] < A[minIdx]) minIdx = j;
        }
        if (minIdx != i) swapInt(&A[i], &A[minIdx]);
    }
}

/* ---------------------------------------------------------------------
   PART B: HEAP SORT  (in-place, using a MAX-heap, ascending output)
   --------------------------------------------------------------------- */
void siftDown(int A[], int n, int i) {
    while (1) {
        int largest = i, left = 2*i + 1, right = 2*i + 2;
        if (left < n)  { g_cmp++; if (A[left]  > A[largest]) largest = left;  }
        if (right < n) { g_cmp++; if (A[right] > A[largest]) largest = right; }
        if (largest == i) break;
        swapInt(&A[i], &A[largest]);
        i = largest;
    }
}
void heapSort(int A[], int n) {
    for (int i = n/2 - 1; i >= 0; i--) siftDown(A, n, i);   /* build max-heap */
    for (int end = n - 1; end > 0; end--) {
        swapInt(&A[0], &A[end]);        /* move current max to the end */
        siftDown(A, end, 0);            /* restore heap on the shrunk range */
    }
}

/* ---------------------------------------------------------------------
   Test harness
   --------------------------------------------------------------------- */
int isSorted(int A[], int n) {
    for (int i = 1; i < n; i++) if (A[i-1] > A[i]) return 0;
    return 1;
}
void makeSorted(int A[], int n)        { for (int i=0;i<n;i++) A[i]=i; }
void makeReverseSorted(int A[], int n) { for (int i=0;i<n;i++) A[i]=n-1-i; }
void makeRandom(int A[], int n) {
    for (int i=0;i<n;i++) A[i]=i;
    for (int i=n-1;i>0;i--) { int j=rand()%(i+1); int t=A[i];A[i]=A[j];A[j]=t; }
}

int main(void) {
    srand(123);

    /* ---- 1. Correctness ---- */
    int fails = 0;
    for (int n = 0; n <= 200; n++) {
        int *A = malloc(sizeof(int)*(n>0?n:1));
        makeRandom(A, n);
        selectionSort(A, n);
        if (!isSorted(A, n)) { fails++; printf("selectionSort FAIL n=%d\n", n); }
        makeRandom(A, n);
        heapSort(A, n);
        if (!isSorted(A, n)) { fails++; printf("heapSort FAIL n=%d\n", n); }
        free(A);
    }
    printf("Correctness check n=0..200: %d failures\n\n", fails);

    /* ---- 2. Selection sort: comparisons/swaps by INPUT ORDER, fixed n ---- */
    FILE *fp1 = fopen("q6_order_data.csv", "w");
    if (!fp1) { perror("fopen"); return 1; }
    fprintf(fp1, "n,order,comparisons,swaps\n");
    int testNs[] = {10, 50, 100, 500, 1000};
    const char *orders[] = {"sorted", "reverse", "random"};
    for (unsigned s = 0; s < sizeof(testNs)/sizeof(testNs[0]); s++) {
        int n = testNs[s];
        int *A = malloc(sizeof(int)*n);
        for (int o = 0; o < 3; o++) {
            if (o==0) makeSorted(A,n);
            else if (o==1) makeReverseSorted(A,n);
            else makeRandom(A,n);
            g_cmp = 0; g_swap = 0;
            selectionSort(A, n);
            fprintf(fp1, "%d,%s,%lld,%lld\n", n, orders[o], g_cmp, g_swap);
            printf("SelectionSort  n=%5d  order=%-8s comparisons=%8lld  swaps=%6lld\n",
                   n, orders[o], g_cmp, g_swap);
        }
        free(A);
        printf("\n");
    }
    fclose(fp1);

    /* ---- 3. Growth comparison: Selection sort vs Heap sort, comparisons vs n ---- */
    FILE *fp2 = fopen("q6_growth_data.csv", "w");
    if (!fp2) { perror("fopen"); return 1; }
    fprintf(fp2, "n,selection_cmp,heap_cmp,n_squared_half,nlogn\n");
    int sizes[] = {10,20,50,100,200,500,1000,2000,5000,10000,20000,50000,100000};
    for (unsigned s = 0; s < sizeof(sizes)/sizeof(sizes[0]); s++) {
        int n = sizes[s];
        int *A = malloc(sizeof(int)*n);

        makeRandom(A, n);
        g_cmp = 0; selectionSort(A, n); long long selCmp = g_cmp;

        makeRandom(A, n);
        g_cmp = 0; heapSort(A, n); long long heapCmp = g_cmp;

        double nsq = n*(n-1)/2.0;
        double nlogn = n * (log(n)/log(2));

        fprintf(fp2, "%d,%lld,%lld,%.1f,%.1f\n", n, selCmp, heapCmp, nsq, nlogn);
        printf("Growth  n=%7d  selectionSort_cmp=%9lld  heapSort_cmp=%9lld\n",
               n, selCmp, heapCmp);
        free(A);
    }
    fclose(fp2);
    return 0;
}