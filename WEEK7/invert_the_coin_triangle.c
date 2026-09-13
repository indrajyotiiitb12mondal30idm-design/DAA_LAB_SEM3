#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAXPTS 4000000

typedef struct { int x, y; } Point;

static int build_up(int n, Point *pts) {
    int k = 0;
    for (int r = 0; r < n; r++)
        for (int c = 0; c <= r; c++) { pts[k].x = 2*c - r; pts[k].y = r; k++; }
    return k;
}
static int build_down(int n, Point *pts, int dx, int dy) {
    int k = 0;
    for (int r = 0; r < n; r++) {
        int width = n - r;
        for (int c = 0; c < width; c++) {
            pts[k].x = 2*c - (width - 1) + dx;
            pts[k].y = r + dy;
            k++;
        }
    }
    return k;
}

int main(int argc, char *argv[]) {
    int n = (argc > 1) ? atoi(argv[1]) : 4;
    if (n < 1) { printf("n must be >= 1\n"); return 1; }

    /* --- Step 1: O(1) analytic optimal shift ------------------------- */
    int dy = (n - 1) / 3;                 /* integer division = floor()   */
    int dx = (n % 3 == 2) ? -1 : 0;

    /* --- Step 2: O(n^2) construction of both point sets ---------------*/
    clock_t tstart = clock();
    long total = (long)n * (n + 1) / 2;
    Point *up   = malloc(sizeof(Point) * total);
    Point *down = malloc(sizeof(Point) * total);
    if (!up || !down) { printf("out of memory\n"); return 1; }
    build_up(n, up);
    build_down(n, down, dx, dy);

    /* --- Step 3: O(n^2) membership test using a direct-address table --
       x ranges roughly over [-n, n], y over [0, n-1] (before/after shift,
       still within a constant multiple of n), so a 2-D boolean grid gives
       O(1) lookups with no hashing needed.                               */
    int W = 4 * n + 10;                    /* width  of the grid (x-axis) */
    int H = 2 * n + 10;                    /* height of the grid (y-axis) */
    int xoff = 2 * n + 5;                  /* shift x into [0, W)          */
    char *grid = calloc((size_t)W * H, 1);
    if (!grid) { printf("out of memory\n"); return 1; }

    #define IDX(x,y) (((long)(y) * W) + ((x) + xoff))

    for (long i = 0; i < total; i++) grid[IDX(down[i].x, down[i].y)] = 1;

    long overlap = 0;
    for (long i = 0; i < total; i++)
        if (grid[IDX(up[i].x, up[i].y)]) overlap++;

    long moves = total - overlap;
    long formula = (long)n * (n + 1) / 6;      /* floor() via integer division */
    clock_t tend = clock();
    double secs = (double)(tend - tstart) / CLOCKS_PER_SEC;
    printf("Internal compute time (clock()): %.6f s\n", secs);

    printf("Rows (n)                 : %d\n", n);
    printf("Total coins  T           : %ld\n", total);
    printf("Analytic optimal shift   : dx=%d, dy=%d   (O(1) -- no search!)\n", dx, dy);
    printf("Overlap at that shift    : %ld\n", overlap);
    printf("Minimum coins to move    : %ld\n", moves);
    printf("Compact formula floor(n(n+1)/6): %ld\n", formula);
    printf("Match?                   : %s\n", moves == formula ? "YES" : "NO");

    /* --- Step 4: emit the explicit move list (still O(n^2)) ------------*/
    printf("\nFirst few required moves (coin -> empty target slot):\n");
    int shown = 0;
    for (long i = 0; i < total && shown < 10; i++) {
        if (!grid[IDX(up[i].x, up[i].y)]) {
            printf("  (%3d,%3d) -> some empty slot of the shifted inverted triangle\n",
                   up[i].x, up[i].y);
            shown++;
        }
    }
    if (moves > 10) printf("  ... (%ld total)\n", moves);

    free(up); free(down); free(grid);
    return 0;
}

/*
Time Complexity: Theta(n^2)
Space Complexity: Theta(n^2)
*/