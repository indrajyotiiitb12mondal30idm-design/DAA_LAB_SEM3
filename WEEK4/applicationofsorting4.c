#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int time;
    int type;   
} Event;

int cmpEvent(const void *a, const void *b) {
    const Event *e1 = (const Event *)a, *e2 = (const Event *)b;
    return e1->time - e2->time;  
}

int main(void) {
    int n;
    printf("Enter number of persons: ");
    if (scanf("%d", &n) != 1 || n <= 0) return 0;

    Event *events = (Event *) malloc(2 * n * sizeof(Event));

    printf("Enter entry and exit time for each person:\n");
    for (int i = 0; i < n; i++) {
        int a, b;
        scanf("%d %d", &a, &b);
        events[2 * i].time     = a;  events[2 * i].type     = +1;
        events[2 * i + 1].time = b;  events[2 * i + 1].type = -1;
    }

    qsort(events, 2 * n, sizeof(Event), cmpEvent); 

    int count = 0, maxCount = 0, bestTime = events[0].time;
    for (int i = 0; i < 2 * n; i++) {
        count += events[i].type;

        if (events[i].type == +1 && count > maxCount) {
            maxCount = count;
            bestTime = events[i].time;
        }
    }

    printf("Maximum simultaneous attendance = %d, achieved at time = %d\n",
           maxCount, bestTime);

    free(events);
    return 0;
}