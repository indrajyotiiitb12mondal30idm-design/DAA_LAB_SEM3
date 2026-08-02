#include <stdio.h>
#include <stdbool.h>

bool has_duplicate_bruteforce(int arr[], int n) {
for (int i = 0; i < n; i++) 
{
    for (int j = i + 1; j < n; j++) 
    {
        if (arr[i] == arr[j]) 
        {
            return true;  
        }
    }
}
return false; 
}

int main()
{
    int n;

    printf("Enter the number of elements: ");
    scanf("%d", &n);

    int arr[n];

    printf("Enter %d elements:\n", n);

    for (int i = 0; i < n; i++)
    {
        scanf("%d", &arr[i]);
    }

    if (has_duplicate_bruteforce(arr, n))
    {
        printf("Duplicate element found.\n");
    }
    else
    {
        printf("No duplicate element found.\n");
    }

    return 0;
}

//CONCLUSION: The brute-force approach has a time complexity of O(n^2) due to the nested loops, making it inefficient for large datasets.