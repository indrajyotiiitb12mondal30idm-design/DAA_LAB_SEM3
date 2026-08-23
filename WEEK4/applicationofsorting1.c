#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int  number;
    char color;   /* 'R', 'B', or 'Y' */
} Item;

void sortByColorStable(Item arr[], int n) {
    int countR = 0, countB = 0, countY = 0;

    /* count how many of each colour there are */
    for (int i = 0; i < n; i++) {
        if      (arr[i].color == 'R') countR++;
        else if (arr[i].color == 'B') countB++;
        else                          countY++;
    }

    int startR = 0;
    int startB = countR;
    int startY = countR + countB;

    int posR = startR, posB = startB, posY = startY;

    Item *output = (Item *) malloc(n * sizeof(Item));

    for (int i = 0; i < n; i++) {
        if      (arr[i].color == 'R') output[posR++] = arr[i];
        else if (arr[i].color == 'B') output[posB++] = arr[i];
        else                          output[posY++] = arr[i];
    }

    for (int i = 0; i < n; i++) arr[i] = output[i];
    free(output);
}

int main(void) {
    int n;
    printf("Enter number of items: ");
    if (scanf("%d", &n) != 1 || n <= 0) return 0;

    Item *arr = (Item *) malloc(n * sizeof(Item));
    printf("Enter %d pairs as: <number> <colour R/B/Y>\n", n);
    for (int i = 0; i < n; i++) {
        char c;
        scanf("%d %c", &arr[i].number, &c);
        arr[i].color = c;
    }

    sortByColorStable(arr, n);

    printf("\nGrouped by colour (Reds, then Blues, then Yellows):\n");
    for (int i = 0; i < n; i++)
        printf("(%d, %c)\n", arr[i].number, arr[i].color);

    free(arr);
    return 0;
}