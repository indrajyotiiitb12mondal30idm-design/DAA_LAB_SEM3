#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define COUNT 12

typedef struct {
 char name[32];
 double log_value; 
} FuncVal;

int compare(const void *a, const void *b) {
    double va = ((FuncVal *)a)->log_value;
    double vb = ((FuncVal *)b)->log_value;
    if (va < vb) return -1;
    if (va > vb) return 1;
    return 0;
}

int main(void) {
    double n = 1e6;
    double ln_n = log(n);
    double log2n = log2(n);
    FuncVal f[COUNT] = {
        {"1/n", -ln_n},
        {"log2(n)", log(log2n)},
        {"12*sqrt(n)", log(12.0) + 0.5 * ln_n},
        {"50*sqrt(n)", log(50.0) + 0.5 * ln_n},
        {"n^0.51", 0.51 * ln_n},
        {"n*log2(n)", ln_n + log(log2n)},
        {"n^2 - 324", log(n * n - 324.0)},
        {"100n^2 + 6n", log(100.0 * n * n + 6.0 * n)},
        {"2n^3", log(2.0) + 3.0 * ln_n},
        {"n^(log2 n)", log2n * ln_n},
        {"3^n", n * log(3.0)},
        {"2^(32n)", 32.0 * n * log(2.0)}
};

qsort(f, COUNT, sizeof(FuncVal), compare);

printf("Increasing order of growth (n = %.0f), compared via ln(f(n)):\n\n", n);
for (int i = 0; i < COUNT; i++) {
    printf("%2d. %-15s ln(value) = %.4f\n", i + 1, f[i].name, f[i].log_value);
}
return 0;
}