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

int main(void) {
    int n, k;
    
    printf("Enter the number of elements N: ");
    if (scanf("%d", &n) != 1 || n <= 0) {
        printf("Please enter a positive integer.\n");
        return 1;
    }
    
    printf("Enter K (1=smallest, N=largest): ");
    if (scanf("%d", &k) != 1 || k < 1 || k > n) {
        printf("K must be between 1 and N.\n");
        return 1;
    }

    int *arr = (int *)malloc((size_t)n * sizeof(int));
    srand((unsigned int)time(NULL));
    
    for (int i = 0; i < n; i++) {
        arr[i] = rand() % 1000;
    }

    if (n <= 30) {
        printf("Unsorted input: ");
        for (int i = 0; i < n; i++) {
            printf("%d ", arr[i]);
        }
        printf("\n");
    }

    clock_t start = clock();
    int result = quickSelect(arr, 0, n - 1, k);
    clock_t end = clock();

    printf("The %d-th smallest element is %d\n", k, result);
    printf("Time taken = %.6f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);

    free(arr);
    return 0;
}

/*
Time Complexity (Average/Best Case): O(N)
Time Complexity (Worst Case): O(N^2)
Space Complexity: O(1)
*/