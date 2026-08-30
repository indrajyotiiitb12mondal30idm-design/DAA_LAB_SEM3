#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define INPUT_FILE "input.txt"
#define OUTPUT_FILE "sorted_output.txt"
#define MAX_VALUE 100000

void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

void generateRandomFile(const char *filename, int n) {
    FILE *fp = fopen(filename, "w");
    if (fp == NULL) { 
        printf("Error opening %s\n", filename); 
        exit(1); 
    }
    srand((unsigned int)time(NULL));
    for (int i = 0; i < n; i++) {
        fprintf(fp, "%d\n", rand() % MAX_VALUE);
    }
    fclose(fp);
}

int* readNumbersFromFile(const char *filename, int *outCount) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("Error: could not open %s for reading.\n", filename);
        exit(1);
    }
    
    int capacity = 16, count = 0;
    int *arr = (int *)malloc((size_t)capacity * sizeof(int));
    int value;
    
    while (fscanf(fp, "%d", &value) == 1) {
        if (count == capacity) {
            capacity *= 2;
            arr = (int *)realloc(arr, (size_t)capacity * sizeof(int));
        }
        arr[count++] = value;
    }
    
    fclose(fp);
    *outCount = count;
    return arr;
}

void writeNumbersToFile(const char *filename, int arr[], int n) {
    FILE *fp = fopen(filename, "w");
    if (fp == NULL) {
        printf("Error: could not open %s for writing.\n", filename);
        exit(1);
    }
    
    for (int i = 0; i < n; i++) {
        fprintf(fp, "%d\n", arr[i]);
    }
    fclose(fp);
}

void heapify(int arr[], int heapSize, int i) {
    int largest = i;
    int left = 2 * i + 1;
    int right = 2 * i + 2;

    if (left < heapSize && arr[left] > arr[largest])
        largest = left;
        
    if (right < heapSize && arr[right] > arr[largest])
        largest = right;

    if (largest != i) {
        swap(&arr[i], &arr[largest]);
        heapify(arr, heapSize, largest);
    }
}

void heapSort(int arr[], int n) {
    for (int i = n / 2 - 1; i >= 0; i--) {
        heapify(arr, n, i);
    }

    for (int end = n - 1; end > 0; end--) {
        swap(&arr[0], &arr[end]);
        heapify(arr, end, 0);
    }
}

int isSorted(int arr[], int n) {
    for (int i = 1; i < n; i++) {
        if (arr[i - 1] > arr[i]) return 0;
    }
    return 1;
}

int main(void) {
    int n;
    
    printf("Enter the number of random elements N to generate: ");
    if (scanf("%d", &n) != 1 || n <= 0) {
        printf("Please enter a positive integer.\n");
        return 1;
    }

    generateRandomFile(INPUT_FILE, n);
    printf("Generated %d random integers and stored them in \"%s\".\n", n, INPUT_FILE);

    int count;
    int *arr = readNumbersFromFile(INPUT_FILE, &count);
    printf("Read %d integers back from \"%s\".\n", count, INPUT_FILE);

    clock_t start = clock();
    heapSort(arr, count);
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;

    writeNumbersToFile(OUTPUT_FILE, arr, count);
    printf("Sorted %d integers in %.6f seconds. Result written to \"%s\".\n", count, elapsed, OUTPUT_FILE);
    printf("Sanity check - is the output actually sorted? %s\n", isSorted(arr, count) ? "YES" : "NO");

    if (count <= 20) {
        printf("Sorted output: ");
        for (int i = 0; i < count; i++) {
            printf("%d ", arr[i]);
        }
        printf("\n");
    }

    free(arr);
    return 0;
}