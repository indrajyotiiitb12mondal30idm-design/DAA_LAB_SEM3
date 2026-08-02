#include <stdio.h>
int find_partition_binary(int A[], int n) {
int lo = 0, hi = n;  
while (lo < hi) {
    int mid = lo + (hi - lo) / 2;
    if (A[mid] == 1) {
    hi = mid; } 
    else {
    lo = mid + 1;  
    }
 }
return lo; 
}

int main(void) {
    int A1[] = {0,0,0,0,0,1,1,1};
    int n1 = sizeof(A1)/sizeof(A1[0]);
    int index = find_partition_binary(A1, n1);
    printf("A1 partition index: %d \n", find_partition_binary(A1, n1));
    if(index !=-1)
    {
        printf("Partition point found at index: %d\n", index);
    }
    else
    {
        printf("No partition point found.\n");
    }
    return 0;
}
