#include <stdio.h>
#include <stdlib.h>
#include <time.h>
long bubble_sort_adaptive(int arr[], int n) {
    long comparisons = 0;
    for (int pass = 0; pass < n - 1; pass++) {
        int swapped = 0;
        for (int j = 0; j < n - 1 - pass; j++) {
            comparisons++;
            if (arr[j] > arr[j + 1]) {
                int t = arr[j]; arr[j] = arr[j + 1]; arr[j + 1] = t;
                swapped = 1;
            }
        }    
        if (!swapped) break;
    }
    return comparisons;
}

long bubble_sort_nonadaptive(int arr[], int n) {
    long comparisons = 0;
    for (int pass = 0; pass < n - 1; pass++) {
        for (int j = 0; j < n - 1 - pass; j++) {
            comparisons++;
            if (arr[j] > arr[j + 1]) {
                int t = arr[j]; arr[j] = arr[j + 1]; arr[j + 1] = t;
            }
        }
   }
   return comparisons;
}

void fill_random(int arr[], int n) {
    for (int i = 0; i < n; i++) arr[i] = rand() % 100000;
}

int main(void) {
    srand((unsigned int) time(NULL));
    int sizes[] = {10, 50, 100, 200, 400, 600, 800, 1000, 1500, 2000};
    int num_sizes = sizeof(sizes) / sizeof(sizes[0]);
    int trials = 20; 
    FILE *fp = fopen("bubble_sort_data.csv", "w");
    fprintf(fp, "n,adaptive_comparisons,nonadaptive_comparisons\n");

    for (int s = 0; s < num_sizes; s++) {
        int n = sizes[s];
        long total_adaptive = 0, total_nonadaptive = 0;

        for (int t = 0; t < trials; t++) {
            int *arr1 = malloc(n * sizeof(int));
            int *arr2 = malloc(n * sizeof(int));
            fill_random(arr1, n);
            for (int i = 0; i < n; i++) arr2[i] = arr1[i]; 
            total_adaptive += bubble_sort_adaptive(arr1, n);
            total_nonadaptive += bubble_sort_nonadaptive(arr2, n);

            free(arr1);
            free(arr2);
        }
        fprintf(fp, "%d,%.1f,%.1f\n", n,(double) total_adaptive / trials,(double) total_nonadaptive / trials);
        printf("n=%5d avg adaptive=%.0f avg nonadaptive=%.0f\n",n, (double) total_adaptive / trials, (double) total_nonadaptive / trials); 
    }
    fclose(fp);
    return 0;
}