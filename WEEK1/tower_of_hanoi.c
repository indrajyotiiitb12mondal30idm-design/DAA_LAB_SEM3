#include<stdio.h>
long long moves = 0 ;
void towerofhanoi(int n , char starting_rod , char helping_rod , char destination_rod)
{
    if(n==1)
    {
        printf("Move disc 1 from %c to %c\n", starting_rod , destination_rod);
        moves++;
    }
    else
    {
        towerofhanoi(n-1 , starting_rod , destination_rod , helping_rod);
        printf("Move disc %d from %c to %c \n", n , starting_rod , destination_rod);
        moves ++;
        towerofhanoi(n-1 , helping_rod , starting_rod , destination_rod);
    }
}

int main()
{
    int n; 
    long long total_moves = 0;
    FILE *fp = fopen("hanoi_data.csv", "w");

    fprintf(fp, "n,moves\n");

    for (int n = 1; n <= 15; n++)
    {
        moves = 0;
        towerofhanoi(n, 'A', 'B', 'C');

        fprintf(fp, "%d,%lld\n", n, moves);

        printf("n=%2d -> moves=%lld\n", n, moves);
        total_moves += moves;
    }

    fclose(fp);
    

    printf("Total number of moves involved in the whole process of tower of hanoi problem is: %lld\n", total_moves);
    return 0;

}

/*CONCLUSION: 
The Tower of Hanoi algorithm follows the recurrence relation:
T(n)=2T(n−1)+1
The total number of moves required is: T(n)=2^n -1 
The plot shows exponential growth, meaning the number of moves approximately doubles whenever one more disk is added.
Therefore, the time complexity of the Tower of Hanoi algorithm is:O(2^n)
Since the running time grows exponentially, the algorithm is practical only for small values of n. Even a modest increase in the number of disks causes a very large increase in the required number of moves.*/