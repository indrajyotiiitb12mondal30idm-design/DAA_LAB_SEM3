#include <stdio.h>
#include <stdlib.h>

int cmpInt(const void *a, const void *b) {
    return (*(const int *)a - *(const int *)b);
}

int binarySearch(const int arr[], int n, int target) {
    int lo = 0, hi = n - 1;
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        if (arr[mid] == target)      return mid;
        else if (arr[mid] < target)  lo = mid + 1;
        else                         hi = mid - 1;
    }
    return -1;
}

int main(void) {
    int n, x;
    printf("Enter size n of each set: ");
    if (scanf("%d", &n) != 1 || n <= 0) return 0;

    int *S1 = (int *) malloc(n * sizeof(int));
    int *S2 = (int *) malloc(n * sizeof(int));

    printf("Enter %d elements of S1:\n", n);
    for (int i = 0; i < n; i++) scanf("%d", &S1[i]);

    printf("Enter %d elements of S2:\n", n);
    for (int i = 0; i < n; i++) scanf("%d", &S2[i]);

    printf("Enter target sum x: ");
    scanf("%d", &x);

  
    qsort(S2, n, sizeof(int), cmpInt);

    int found = 0;
    for (int i = 0; i < n; i++) {
        int need = x - S1[i];
        int idx = binarySearch(S2, n, need);
        if (idx != -1) {
            printf("YES: %d (from S1) + %d (from S2) = %d\n", S1[i], S2[idx], x);
            found = 1;
            break;  
        }
    }
    if (!found) printf("NO such pair exists.\n");

    free(S1);
    free(S2);
    return 0;
}