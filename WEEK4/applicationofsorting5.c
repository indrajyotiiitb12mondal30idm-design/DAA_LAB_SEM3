#include <stdio.h>
#include <stdlib.h>

typedef struct { int x, y; } Interval;

int cmpInterval(const void *a, const void *b) {
    const Interval *i1 = (const Interval *)a, *i2 = (const Interval *)b;
    return i1->x - i2->x;   
}

int main(void) {
    int n;
    printf("Enter number of intervals: ");
    if (scanf("%d", &n) != 1 || n <= 0) return 0;

    Interval *arr = (Interval *) malloc(n * sizeof(Interval));
    printf("Enter %d intervals as: <x> <y>\n", n);
    for (int i = 0; i < n; i++) scanf("%d %d", &arr[i].x, &arr[i].y);

    qsort(arr, n, sizeof(Interval), cmpInterval);   

    Interval *result = (Interval *) malloc(n * sizeof(Interval));
    int rcount = 0;

    Interval cur = arr[0];
    for (int i = 1; i < n; i++) {
        if (arr[i].x <= cur.y) {
            if (arr[i].y > cur.y) cur.y = arr[i].y;
        } else {
            result[rcount++] = cur;
            cur = arr[i];
        }
    }
    result[rcount++] = cur;   

    printf("Merged intervals:\n");
    for (int i = 0; i < rcount; i++)
        printf("(%d, %d)\n", result[i].x, result[i].y);

    free(arr);
    free(result);
    return 0;
}