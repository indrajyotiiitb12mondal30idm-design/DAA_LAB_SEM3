#include <stdio.h>
#include <stdlib.h>

typedef struct { int coord; int type; } Event;   

int cmpEvent(const void *a, const void *b) {
    const Event *e1 = (const Event *)a, *e2 = (const Event *)b;
    if (e1->coord != e2->coord) return e1->coord - e2->coord;
    
    return e1->type - e2->type;
}

int main(void) {
    int n;
    printf("Enter number of intervals: ");
    if (scanf("%d", &n) != 1 || n <= 0) return 0;

    Event *events = (Event *) malloc(2 * n * sizeof(Event));
    printf("Enter %d intervals as: <l> <r>\n", n);
    for (int i = 0; i < n; i++) {
        int l, r;
        scanf("%d %d", &l, &r);
        events[2 * i].coord     = l;  events[2 * i].type     = 0;
        events[2 * i + 1].coord = r;  events[2 * i + 1].type = 1;
    }

    qsort(events, 2 * n, sizeof(Event), cmpEvent);  
    int count = 0, maxCount = 0, bestPoint = events[0].coord;
    for (int i = 0; i < 2 * n; i++) {
        if (events[i].type == 0) {         
            count++;
            if (count > maxCount) { maxCount = count; bestPoint = events[i].coord; }
        } else {                       
            count--;
        }
    }

    printf("Point with max coverage: p = %d, covered by %d intervals\n",
           bestPoint, maxCount);

    free(events);
    return 0;
}