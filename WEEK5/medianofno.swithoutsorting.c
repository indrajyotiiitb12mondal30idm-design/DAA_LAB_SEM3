#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

int partition(int arr[], int low, int high) {
    int pivot = arr[high];
    int i = low - 1;
    for (int j = low; j < high; j++) {
        if (arr[j] <= pivot) {
            i++;
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[i + 1], &arr[high]);
    return i + 1;
}

int quickSelect(int arr[], int low, int high, int k) {
    if (low == high) return arr[low];

    int pivotIndex = partition(arr, low, high);
    int rank = pivotIndex - low + 1; 

    if (k == rank)
        return arr[pivotIndex];
    else if (k < rank)
        return quickSelect(arr, low, pivotIndex - 1, k);
    else
        return quickSelect(arr, pivotIndex + 1, high, k - rank);
}

double findMedian(int arr[], int n) {
    if (n % 2 != 0) {
        int k = (n + 1) / 2;
        return (double) quickSelect(arr, 0, n - 1, k);
    } else {
        
        int k = n / 2;
        int lowerMid = quickSelect(arr, 0, n - 1, k);

        
        int upperMid = arr[k];
        for (int i = k + 1; i < n; i++)
            if (arr[i] < upperMid)
                upperMid = arr[i];

        return (lowerMid + upperMid) / 2.0;
    }
}

int main(void) {
    int n;
    printf("Enter the number of elements N: ");
    if (scanf("%d", &n) != 1 || n <= 0) {
        printf("Please enter a positive integer.\n");
        return 1;
    }

    int *arr = (int *) malloc((size_t) n * sizeof(int));
    srand((unsigned int) time(NULL));
    for (int i = 0; i < n; i++)
        arr[i] = rand() % 1000;  

    if (n <= 30) {
        printf("Unsorted input: ");
        for (int i = 0; i < n; i++) printf("%d ", arr[i]);
        printf("\n");
    }

    clock_t start = clock();
    double median = findMedian(arr, n);
    clock_t end = clock();

    printf("Median = %.2f\n", median);
    printf("Time taken = %.6f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);

    free(arr);
    return 0;
}


/*

Time Complexity

Worst-Case: O(N²)

Average-Case: O(N)

Best-Case: O(N)

Space / Auxiliary Complexity

Auxiliary Space (Call Stack): O(N) in the worst case, O(log N) in the average case.

Overall Memory: O(N) to store the dynamically allocated array in main.

*/