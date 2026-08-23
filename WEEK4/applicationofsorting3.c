#include <stdio.h>
#include <stdlib.h>

#define MAXN 1000
#define MAXK 20

int arr[MAXN];
int n, k;
long long T;
int chosenIdx[MAXK];

int cmpInt(const void *a, const void *b) {
    return (*(const int *)a - *(const int *)b);
}

int binSearchRange(int lo, int hi, long long target) {
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        if (arr[mid] == target)      return mid;
        else if (arr[mid] < target)  lo = mid + 1;
        else                         hi = mid - 1;
    }
    return -1;
}

int findK(int start, int chosenCount, long long sumSoFar) {
    if (chosenCount == k - 1) {
        long long remainder = T - sumSoFar;
        int idx = binSearchRange(start, n - 1, remainder);
        if (idx != -1) {
            chosenIdx[chosenCount] = idx;
            return 1;
        }
        return 0;
    }

    int lastStart = n - (k - chosenCount);
    for (int i = start; i <= lastStart; i++) {
        chosenIdx[chosenCount] = i;
        if (findK(i + 1, chosenCount + 1, sumSoFar + arr[i]))
            return 1;
    }
    return 0;
}

int main(void) {
    printf("Enter n (set size) and k (how many must sum to T): ");
    if (scanf("%d %d", &n, &k) != 2 || n <= 0 || k <= 0 || k > n) return 0;

    printf("Enter %d integers of S:\n", n);
    for (int i = 0; i < n; i++) scanf("%d", &arr[i]);

    printf("Enter target sum T: ");
    scanf("%lld", &T);

    qsort(arr, n, sizeof(int), cmpInt); 

    if (findK(0, 0, 0)) {
        printf("YES: found %d numbers that sum to %lld -> ", k, T);
        for (int i = 0; i < k; i++) printf("%d ", arr[chosenIdx[i]]);
        printf("\n");
    } else {
        printf("NO: no %d numbers in S sum to %lld\n", k, T);
    }
    return 0;
}