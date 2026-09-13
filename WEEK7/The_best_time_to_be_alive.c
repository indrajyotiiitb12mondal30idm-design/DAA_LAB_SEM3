#include <stdio.h>
#include <stdlib.h>

typedef struct { int year; int delta; int isBirth; } Event;

int cmp(const void *a, const void *b) {
    const Event *ea = a, *eb = b;
    if (ea->year != eb->year) return ea->year - eb->year;
    return ea->isBirth - eb->isBirth;
}

int main(int argc, char *argv[]) {
    int demoBirths[] = {1900, 1850, 1910, 1875, 1905, 1860, 1920, 1830};
    int demoDeaths[] = {1970, 1920, 1980, 1940, 1975, 1900, 1990, 1905};
    int n;
    int *birth, *death;

    if (argc > 1) {
        n = (argc - 1) / 2;
        birth = malloc(sizeof(int) * n);
        death = malloc(sizeof(int) * n);
        for (int i = 0; i < n; i++) {
            birth[i] = atoi(argv[1 + 2*i]);
            death[i] = atoi(argv[2 + 2*i]);
        }
    } else {
        n = sizeof(demoBirths) / sizeof(demoBirths[0]);
        birth = demoBirths; death = demoDeaths;
    }

    Event *ev = malloc(sizeof(Event) * 2 * n);
    for (int i = 0; i < n; i++) {
        ev[2*i]   = (Event){ birth[i], +1, 1 };
        ev[2*i+1] = (Event){ death[i], -1, 0 };
    }
    qsort(ev, 2 * n, sizeof(Event), cmp);

    int alive = 0, best = -1, bestYear = 0;
    for (int k = 0; k < 2 * n; k++) {
        alive += ev[k].delta;
        if (ev[k].isBirth && alive > best) { best = alive; bestYear = ev[k].year; }
    }

    printf("Number of scientists (n)         : %d\n", n);
    printf("Maximum simultaneously alive     : %d\n", best);
    printf("First year that maximum is hit   : %d\n", bestYear);

    free(ev);
    if (argc > 1) { free(birth); free(death); }
    return 0;
}

/*
Time Complexity: O(nlog n)
Space Complexity: Theta(n)
*/