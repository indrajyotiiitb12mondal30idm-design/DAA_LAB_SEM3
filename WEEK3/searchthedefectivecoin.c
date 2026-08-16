#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GENUINE 100
#define LIGHTER 99
#define MAXN 200005

int trueWeights[MAXN];        /* ground truth, hidden from the algorithm */
long long g_weighings;        /* counts calls to weigh()                 */

typedef enum { LEFT_LIGHTER, RIGHT_LIGHTER, BALANCED } ScaleResult;

/* The balance scale: compares the SUM of weights of two EQUAL-SIZED
   coin groups given by index arrays. This is the only "oracle" call
   every algorithm is permitted to use. Equal size is enforced by the
   caller - it is what makes the comparison meaningful.                */
ScaleResult weigh(int *left, int *right, int count) {
    g_weighings++;
    long long sumL = 0, sumR = 0;
    for (int i = 0; i < count; i++) sumL += trueWeights[left[i]];
    for (int i = 0; i < count; i++) sumR += trueWeights[right[i]];
    if (sumL < sumR) return LEFT_LIGHTER;
    if (sumR < sumL) return RIGHT_LIGHTER;
    return BALANCED;
}

/* =====================================================================
   (A) BINARY-SPLIT ALGORITHM  -  target: floor(log2 n) + c weighings
   ===================================================================== */

/* Precondition: exactly ONE defective coin is known to exist among
   coins[0..m-1]. Pure elimination - no reference coin ever needed.    */
int findWithinBinary(int *coins, int m) {
    if (m == 1) return coins[0];                      /* elimination */

    int half = m / 2;
    int *group1 = coins;
    int *group2 = coins + half;

    ScaleResult r = weigh(group1, group2, half);

    if (r == BALANCED)       return coins[m - 1];      /* m is odd; leftover is guilty */
    else if (r == LEFT_LIGHTER)  return findWithinBinary(group1, half);
    else /* RIGHT_LIGHTER */      return findWithinBinary(group2, half);
}

/* Top level: we do NOT yet know whether a defective coin exists at all. */
int findDefectiveBinary(int *coins, int n) {
    if (n <= 1) return -1;

    int half = n / 2;
    int *group1 = coins;
    int *group2 = coins + half;

    ScaleResult r = weigh(group1, group2, half);

    if (r == LEFT_LIGHTER)  return findWithinBinary(group1, half);
    if (r == RIGHT_LIGHTER) return findWithinBinary(group2, half);

    /* balanced: group1 + group2 (2*half coins) are all genuine */
    if (n % 2 == 0) return -1;

    /* one leftover coin was never weighed - test it against ANY
       already-proven-genuine coin, e.g. coins[0]                     */
    int leftover[1] = { coins[n - 1] };
    int reference[1] = { coins[0] };
    ScaleResult r2 = weigh(leftover, reference, 1);
    return (r2 == LEFT_LIGHTER) ? coins[n - 1] : -1;
}

/* =====================================================================
   (B) TERNARY-SPLIT ALGORITHM   ("best": fewer weighings, ~log3 n)
   ===================================================================== */

/* Splits m coins into three groups: two EQUAL groups (sizeAB each) that
   will be weighed against each other, and a third "spare" group
   (sizeC) that is left aside. We deliberately keep sizeC <= sizeAB
   whenever possible - this guarantees that IF the two weighed groups
   balance, the 2*sizeAB coins already certified genuine are enough to
   build a same-size genuine reference set for the spare group (needed
   only by the top-level function; see below). Precondition: m >= 3.  */
static void splitSizes(int m, int *sizeAB, int *sizeC) {
    int s = m / 3, rem = m % 3;
    if (rem == 2) { *sizeAB = s + 1; *sizeC = s; }       /* keep spare SMALL  */
    else          { *sizeAB = s;     *sizeC = s + rem; } /* rem = 0 or 1 */
}

/* Precondition: exactly ONE defective coin exists among coins[0..m-1] */
int findWithinTernary(int *coins, int m) {
    if (m == 1) return coins[0];
    if (m == 2) {
        ScaleResult r = weigh(&coins[0], &coins[1], 1);
        return (r == LEFT_LIGHTER) ? coins[0] : coins[1];
    }
    int sizeAB, sizeC;
    splitSizes(m, &sizeAB, &sizeC);
    int *group1 = coins;                  /* size sizeAB */
    int *group2 = coins + sizeAB;         /* size sizeAB */
    int *group3 = coins + 2 * sizeAB;     /* size sizeC  */

    ScaleResult r = weigh(group1, group2, sizeAB);

    if (r == LEFT_LIGHTER)       return findWithinTernary(group1, sizeAB);
    else if (r == RIGHT_LIGHTER) return findWithinTernary(group2, sizeAB);
    else /* balanced: by precondition, the defect MUST be in group3 */
        return findWithinTernary(group3, sizeC);
}

/* Top level: existence of a defect is NOT yet guaranteed. */
int findDefectiveTernary(int *coins, int n) {
    if (n <= 1) return -1;
    if (n == 2) {
        ScaleResult r = weigh(&coins[0], &coins[1], 1);
        if (r == LEFT_LIGHTER)  return coins[0];
        if (r == RIGHT_LIGHTER) return coins[1];
        return -1;
    }

    int sizeAB, s3;
    splitSizes(n, &sizeAB, &s3);
    int *group1 = coins;
    int *group2 = coins + sizeAB;
    int *group3 = coins + 2 * sizeAB;

    ScaleResult r = weigh(group1, group2, sizeAB);

    if (r == LEFT_LIGHTER)  return findWithinTernary(group1, sizeAB);
    if (r == RIGHT_LIGHTER) return findWithinTernary(group2, sizeAB);

    /* balanced -> group1 and group2 (2*sizeAB coins) are certified
       genuine. group3 (size s3) might, or might not, hold the defect.
       Build a genuine reference set of EXACTLY s3 coins by pulling
       from the 2*sizeAB certified-genuine pool (group1 then group2).
       splitSizes() guarantees s3 <= 2*sizeAB always, so this never
       runs out of genuine coins to borrow.                               */
    int *refFull = malloc(sizeof(int) * s3);
    int idx = 0;
    for (int i = 0; i < sizeAB && idx < s3; i++) refFull[idx++] = group1[i];
    for (int i = 0; i < sizeAB && idx < s3; i++) refFull[idx++] = group2[i];

    ScaleResult r2 = weigh(group3, refFull, s3);
    free(refFull);
    if (r2 == LEFT_LIGHTER) return findWithinTernary(group3, s3);
    return -1;   /* group3 balanced against genuine reference -> no defect */
}

/* =====================================================================
   Correctness test harness
   ===================================================================== */
int runTrial(int n, int defectPos /* -1 = no defect */, int useBinary) {
    int *coins = malloc(sizeof(int) * n);
    for (int i = 0; i < n; i++) { coins[i] = i; trueWeights[i] = GENUINE; }
    if (defectPos >= 0) trueWeights[defectPos] = LIGHTER;

    g_weighings = 0;
    int result = useBinary ? findDefectiveBinary(coins, n)
                            : findDefectiveTernary(coins, n);
    free(coins);
    return (result == defectPos) ? (int)g_weighings : -1000000;
}

int main(void) {
    /* ---------- 1. Exhaustive correctness check ----------
       NOTE: n=1 is intentionally excluded. With a single coin and
       zero reference coins, NO algorithm (ours or any other) can
       ever tell a defective coin from a genuine one - this is a
       fundamental information-theoretic limit, not a bug. The
       problem statement itself implies n-1 >= 1, i.e. n >= 2.    */
    int fails = 0, totalTrials = 0;
    for (int n = 2; n <= 400; n++) {
        for (int defect = -1; defect < n; defect++) {
            totalTrials += 2;
            if (runTrial(n, defect, 1) < 0) { fails++; printf("BINARY FAIL n=%d defect=%d\n", n, defect); }
            if (runTrial(n, defect, 0) < 0) { fails++; printf("TERNARY FAIL n=%d defect=%d\n", n, defect); }
        }
    }
    printf("Correctness check: %d trials, %d failures\n", totalTrials, fails);

    /* ---------- 2. Worst-case weighings vs n (for plotting) ---------- */
    FILE *fp = fopen("q2_data.csv", "w");
    fprintf(fp, "n,binary_worst,ternary_worst\n");
    int sizes[] = {2,3,4,5,8,10,16,20,27,32,50,64,81,100,128,200,243,256,
                   500,512,729,1000,1024,2000,2187,4096,6561,8192,10000,
                   16384,19683,32768,59049,65536,100000,177147};
    for (unsigned s = 0; s < sizeof(sizes)/sizeof(sizes[0]); s++) {
        int n = sizes[s];
        if (n >= MAXN) continue;
        int worstB = 0, worstT = 0;
        for (int defect = -1; defect < n; defect++) {
            int wB = runTrial(n, defect, 1);
            int wT = runTrial(n, defect, 0);
            if (wB > worstB) worstB = wB;
            if (wT > worstT) worstT = wT;
        }
        fprintf(fp, "%d,%d,%d\n", n, worstB, worstT);
        printf("n=%7d  binary_worst_weighings=%2d  ternary_worst_weighings=%2d\n",
               n, worstB, worstT);
    }
    fclose(fp);
    return 0;
}