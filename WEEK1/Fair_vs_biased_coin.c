#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Simulate N tosses of a coin with P(heads) = p;
   return observed proportion of heads */
double simulate_coin(int N, double p)
{
    int heads = 0;

    for (int i = 0; i < N; i++)
    {
        double u = (double) rand() / ((double) RAND_MAX + 1.0);

        /* Uniform random number in [0,1) */
        if (u < p)
            heads++;
    }

    return (double) heads / N;
}

int main(void)
{
    srand((unsigned int) time(NULL));

    int N = 100000;

    double fair_result = simulate_coin(N, 0.5);
    double biased_result = simulate_coin(N, 0.7);

    printf("Fair coin (true p = 0.50): observed P(head) = %.4f\n",
           fair_result);

    printf("Biased coin (true p = 0.70): observed P(head) = %.4f\n",
           biased_result);

    printf("Difference from true probability: fair = %.4f, biased = %.4f\n",
           fair_result - 0.5,
           biased_result - 0.7);

    return 0;
}