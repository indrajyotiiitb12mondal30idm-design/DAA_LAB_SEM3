#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#endif

#define CAP        20000     /* physical capacity reserved for every array / pool  */
#define POOL_SIZE  17000      /* how many distinct keys we pre-shuffle              */
#define REPEATS    300        /* how many times we repeat a timing to average noise */
#define ABSENT_KEY 999999     /* a key guaranteed not to be present -> worst case   */

/* A volatile "sink" forces gcc to actually keep every computed result
   instead of silently deleting a call whose return value is unused
   (a classic micro-benchmarking trap under -O2 optimisation).        */
static volatile long g_sink = 0;

/* ---------------------------------------------------------------------
   Nanosecond resolution stop-watch using the monotonic system clock.
   --------------------------------------------------------------------- */
static double now_ns(void) {
#ifdef _WIN32
    static LARGE_INTEGER freq;
    static BOOL initialized = FALSE;
    if (!initialized) {
        QueryPerformanceFrequency(&freq);
        initialized = TRUE;
    }
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    return (double)counter.QuadPart * 1e9 / (double)freq.QuadPart;
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1e9 + (double)ts.tv_nsec;
#endif
}

/* =====================================================================
   SECTION 1 : UNSORTED ARRAY
   ===================================================================== */
static int ua_search(int *a, int n, int key) {
    for (int i = 0; i < n; i++) if (a[i] == key) return i;
    return -1;
}
static int ua_insert(int *a, int *n, int key) {
    a[*n] = key;
    (*n)++;
    return (*n) - 1;
}
static void ua_delete(int *a, int *n, int idx) {   /* O(1): swap with last element */
    a[idx] = a[(*n) - 1];
    (*n)--;
}
static int ua_max(int *a, int n) {
    int best = 0;
    for (int i = 1; i < n; i++) if (a[i] > a[best]) best = i;
    return best;
}
static int ua_min(int *a, int n) {
    int best = 0;
    for (int i = 1; i < n; i++) if (a[i] < a[best]) best = i;
    return best;
}
static int ua_predecessor(int *a, int n, int idx) {
    int key = a[idx], best = -1;
    for (int i = 0; i < n; i++)
        if (a[i] < key && (best == -1 || a[i] > a[best])) best = i;
    return best;
}
static int ua_successor(int *a, int n, int idx) {
    int key = a[idx], best = -1;
    for (int i = 0; i < n; i++)
        if (a[i] > key && (best == -1 || a[i] < a[best])) best = i;
    return best;
}

/* =====================================================================
   SECTION 2 : SORTED ARRAY  (ascending order maintained at all times)
   ===================================================================== */
static int sa_search(int *a, int n, int key) {
    int lo = 0, hi = n - 1;
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        if (a[mid] == key) return mid;
        if (a[mid] < key) lo = mid + 1; else hi = mid - 1;
    }
    return -1;
}
static int sa_insert(int *a, int *n, int key) {
    int i = *n - 1;
    while (i >= 0 && a[i] > key) { a[i + 1] = a[i]; i--; }
    a[i + 1] = key;
    (*n)++;
    return i + 1;
}
static void sa_delete(int *a, int *n, int idx) {
    for (int i = idx; i < *n - 1; i++) a[i] = a[i + 1];
    (*n)--;
}
static int sa_max(int n)               { return n - 1; }
static int sa_min(void)                { return 0; }
static int sa_predecessor(int n, int idx) { (void)n; return idx == 0 ? -1 : idx - 1; }
static int sa_successor(int n, int idx)   { return idx == n - 1 ? -1 : idx + 1; }

/* =====================================================================
   SECTION 3 : SINGLY LINKED LIST  (one node type, shared by the
   "unsorted" and the "sorted" variant -- only the INSERT routine
   differs between them; delete/search/max/min work on the same node)
   ===================================================================== */
typedef struct SNode {
    int key;
    struct SNode *next;
} SNode;

/* ---- shared helpers ---- */
static SNode* sll_find_last(SNode *head) {         /* O(n) : walk to the tail */
    if (!head) return NULL;
    while (head->next) head = head->next;
    return head;
}
static SNode* sll_find_predecessor(SNode *head, SNode *target) { /* O(n) */
    if (head == target) return NULL;
    while (head && head->next != target) head = head->next;
    return head;
}
static void sll_unlink(SNode **head, SNode *target) {  /* O(n): needs predecessor */
    if (*head == target) { *head = target->next; free(target); return; }
    SNode *pred = sll_find_predecessor(*head, target);
    pred->next = target->next;
    free(target);
}
static void sll_free_all(SNode *head) {
    while (head) { SNode *nx = head->next; free(head); head = nx; }
}

/* ---- 3a. UNSORTED singly linked list ---- */
static SNode* us_search(SNode *head, int key) {
    for (SNode *c = head; c; c = c->next) if (c->key == key) return c;
    return NULL;
}
static SNode* us_insert(SNode **head, int key) {        /* O(1): push at front */
    SNode *node = malloc(sizeof(SNode));
    node->key = key; node->next = *head; *head = node;
    return node;
}
static SNode* us_max(SNode *head) {
    SNode *best = head;
    for (SNode *c = head; c; c = c->next) if (c->key > best->key) best = c;
    return best;
}
static SNode* us_min(SNode *head) {
    SNode *best = head;
    for (SNode *c = head; c; c = c->next) if (c->key < best->key) best = c;
    return best;
}
static SNode* us_predecessor(SNode *head, SNode *target) {
    SNode *best = NULL;
    for (SNode *c = head; c; c = c->next)
        if (c->key < target->key && (!best || c->key > best->key)) best = c;
    return best;
}
static SNode* us_successor(SNode *head, SNode *target) {
    SNode *best = NULL;
    for (SNode *c = head; c; c = c->next)
        if (c->key > target->key && (!best || c->key < best->key)) best = c;
    return best;
}

/* ---- 3b. SORTED singly linked list ---- */
static SNode* ss_search(SNode *head, int key) {
    SNode *c = head;
    while (c && c->key < key) c = c->next;
    return (c && c->key == key) ? c : NULL;
}
static SNode* ss_insert(SNode **head, int key) {         /* O(n): keeps order */
    SNode *node = malloc(sizeof(SNode));
    node->key = key;
    if (!*head || (*head)->key >= key) { node->next = *head; *head = node; return node; }
    SNode *c = *head;
    while (c->next && c->next->key < key) c = c->next;
    node->next = c->next;
    c->next = node;
    return node;
}
/* min = head (O(1)); max = sll_find_last (O(n));
   successor = target->next (O(1)); predecessor = sll_find_predecessor (O(n)) */

/* =====================================================================
   SECTION 4 : DOUBLY LINKED LIST (shared node type; head+tail kept)
   ===================================================================== */
typedef struct DNode {
    int key;
    struct DNode *prev, *next;
} DNode;

static void dll_unlink(DNode **head, DNode **tail, DNode *target) { /* O(1) */
    if (target->prev) target->prev->next = target->next; else *head = target->next;
    if (target->next) target->next->prev = target->prev; else *tail = target->prev;
    free(target);
}
static void dll_free_all(DNode *head) {
    while (head) { DNode *nx = head->next; free(head); head = nx; }
}

/* ---- 4a. UNSORTED doubly linked list ---- */
static DNode* ud_search(DNode *head, int key) {
    for (DNode *c = head; c; c = c->next) if (c->key == key) return c;
    return NULL;
}
static DNode* ud_insert(DNode **head, DNode **tail, int key) {   /* O(1): push at front */
    DNode *node = malloc(sizeof(DNode));
    node->key = key; node->prev = NULL; node->next = *head;
    if (*head) (*head)->prev = node; else *tail = node;
    *head = node;
    return node;
}
static DNode* ud_max(DNode *head) {
    DNode *best = head;
    for (DNode *c = head; c; c = c->next) if (c->key > best->key) best = c;
    return best;
}
static DNode* ud_min(DNode *head) {
    DNode *best = head;
    for (DNode *c = head; c; c = c->next) if (c->key < best->key) best = c;
    return best;
}
static DNode* ud_predecessor(DNode *head, DNode *target) {
    DNode *best = NULL;
    for (DNode *c = head; c; c = c->next)
        if (c->key < target->key && (!best || c->key > best->key)) best = c;
    return best;
}
static DNode* ud_successor(DNode *head, DNode *target) {
    DNode *best = NULL;
    for (DNode *c = head; c; c = c->next)
        if (c->key > target->key && (!best || c->key < best->key)) best = c;
    return best;
}

/* ---- 4b. SORTED doubly linked list ---- */
static DNode* sd_search(DNode *head, int key) {
    DNode *c = head;
    while (c && c->key < key) c = c->next;
    return (c && c->key == key) ? c : NULL;
}
static DNode* sd_insert(DNode **head, DNode **tail, int key) {   /* O(n): keeps order */
    DNode *node = malloc(sizeof(DNode));
    node->key = key;
    if (!*head || (*head)->key >= key) {
        node->prev = NULL; node->next = *head;
        if (*head) (*head)->prev = node; else *tail = node;
        *head = node;
        return node;
    }
    DNode *c = *head;
    while (c->next && c->next->key < key) c = c->next;
    node->next = c->next;
    node->prev = c;
    if (c->next) c->next->prev = node; else *tail = node;
    c->next = node;
    return node;
}
/* min = head (O(1)); max = tail (O(1));
   successor = target->next (O(1)); predecessor = target->prev (O(1)) */


static void shuffle(int *a, int n) {                 
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int t = a[i]; a[i] = a[j]; a[j] = t;
    }
}


int main(void) {
    srand(42);   /* fixed seed => reproducible numbers every run */

    int *pool = malloc(sizeof(int) * POOL_SIZE);
    for (int i = 0; i < POOL_SIZE; i++) pool[i] = i;
    shuffle(pool, POOL_SIZE);

    FILE *fp = fopen("q1_timings.csv", "w");
    if (!fp) { perror("fopen"); return 1; }
    fprintf(fp, "structure,operation,n,avg_time_ns\n");

    int sizes[] = {250, 500, 1000, 2000, 4000, 8000, 16000};
    int num_sizes = (int)(sizeof(sizes) / sizeof(sizes[0]));

    for (int s = 0; s < num_sizes; s++) {
        int n = sizes[s];
        printf("Running experiments for n = %d ...\n", n);

        /* ============ 1. UNSORTED ARRAY ============ */
        {
            int *arr = malloc(sizeof(int) * CAP);
            int sz = 0;
            for (int i = 0; i < n; i++) ua_insert(arr, &sz, pool[i]);
            int fixed_idx = sz / 2;
            double t;

            t = 0; for (int r = 0; r < REPEATS; r++) { double t0 = now_ns();
                g_sink = ua_search(arr, sz, ABSENT_KEY); t += now_ns() - t0; }
            fprintf(fp, "Unsorted Array,Search,%d,%.2f\n", n, t / REPEATS);

            t = 0; for (int r = 0; r < REPEATS; r++) { double t0 = now_ns();
                g_sink = ua_max(arr, sz); t += now_ns() - t0; }
            fprintf(fp, "Unsorted Array,Max,%d,%.2f\n", n, t / REPEATS);

            t = 0; for (int r = 0; r < REPEATS; r++) { double t0 = now_ns();
                g_sink = ua_min(arr, sz); t += now_ns() - t0; }
            fprintf(fp, "Unsorted Array,Min,%d,%.2f\n", n, t / REPEATS);

            t = 0; for (int r = 0; r < REPEATS; r++) { double t0 = now_ns();
                g_sink = ua_predecessor(arr, sz, fixed_idx); t += now_ns() - t0; }
            fprintf(fp, "Unsorted Array,Predecessor,%d,%.2f\n", n, t / REPEATS);

            t = 0; for (int r = 0; r < REPEATS; r++) { double t0 = now_ns();
                g_sink = ua_successor(arr, sz, fixed_idx); t += now_ns() - t0; }
            fprintf(fp, "Unsorted Array,Successor,%d,%.2f\n", n, t / REPEATS);

            t = 0; for (int r = 0; r < REPEATS; r++) { int nk = pool[n + r];
                double t0 = now_ns(); ua_insert(arr, &sz, nk); t += now_ns() - t0; }
            fprintf(fp, "Unsorted Array,Insert,%d,%.2f\n", n, t / REPEATS);

            t = 0; for (int r = 0; r < REPEATS; r++) { int idx = sz - 1;
                double t0 = now_ns(); ua_delete(arr, &sz, idx); t += now_ns() - t0; }
            fprintf(fp, "Unsorted Array,Delete,%d,%.2f\n", n, t / REPEATS);

            free(arr);
        }

        /* ============ 2. SORTED ARRAY ============ */
        {
            int *arr = malloc(sizeof(int) * CAP);
            int sz = 0;
            for (int i = 0; i < n; i++) sa_insert(arr, &sz, pool[i]);
            int fixed_idx = sz / 2;
            double t;

            t = 0; for (int r = 0; r < REPEATS; r++) { double t0 = now_ns();
                g_sink = sa_search(arr, sz, ABSENT_KEY); t += now_ns() - t0; }
            fprintf(fp, "Sorted Array,Search,%d,%.2f\n", n, t / REPEATS);

            t = 0; for (int r = 0; r < REPEATS; r++) { double t0 = now_ns();
                g_sink = sa_max(sz); t += now_ns() - t0; }
            fprintf(fp, "Sorted Array,Max,%d,%.2f\n", n, t / REPEATS);

            t = 0; for (int r = 0; r < REPEATS; r++) { double t0 = now_ns();
                g_sink = sa_min(); t += now_ns() - t0; }
            fprintf(fp, "Sorted Array,Min,%d,%.2f\n", n, t / REPEATS);

            t = 0; for (int r = 0; r < REPEATS; r++) { double t0 = now_ns();
                g_sink = sa_predecessor(sz, fixed_idx); t += now_ns() - t0; }
            fprintf(fp, "Sorted Array,Predecessor,%d,%.2f\n", n, t / REPEATS);

            t = 0; for (int r = 0; r < REPEATS; r++) { double t0 = now_ns();
                g_sink = sa_successor(sz, fixed_idx); t += now_ns() - t0; }
            fprintf(fp, "Sorted Array,Successor,%d,%.2f\n", n, t / REPEATS);

            /* worst-case insert = new MINIMUM every time -> shifts the whole array */
            t = 0; for (int r = 0; r < REPEATS; r++) { int nk = -1 - r;
                double t0 = now_ns(); sa_insert(arr, &sz, nk); t += now_ns() - t0; }
            fprintf(fp, "Sorted Array,Insert,%d,%.2f\n", n, t / REPEATS);

            /* worst-case delete = always remove index 0 -> shifts the whole array */
            t = 0; for (int r = 0; r < REPEATS; r++) {
                double t0 = now_ns(); sa_delete(arr, &sz, 0); t += now_ns() - t0; }
            fprintf(fp, "Sorted Array,Delete,%d,%.2f\n", n, t / REPEATS);

            free(arr);
        }

        /* ============ 3. UNSORTED SINGLY LINKED LIST ============ */
        {
            SNode *head = NULL;
            for (int i = 0; i < n; i++) us_insert(&head, pool[i]);
            SNode *fixed = head;              /* stable while we only read */
            double t;

            t = 0; for (int r = 0; r < REPEATS; r++) { double t0 = now_ns();
                SNode *res = us_search(head, ABSENT_KEY); g_sink = res ? res->key : -1;
                t += now_ns() - t0; }
            fprintf(fp, "Unsorted SLL,Search,%d,%.2f\n", n, t / REPEATS);

            t = 0; for (int r = 0; r < REPEATS; r++) { double t0 = now_ns();
                SNode *res = us_max(head); g_sink = res->key; t += now_ns() - t0; }
            fprintf(fp, "Unsorted SLL,Max,%d,%.2f\n", n, t / REPEATS);

            t = 0; for (int r = 0; r < REPEATS; r++) { double t0 = now_ns();
                SNode *res = us_min(head); g_sink = res->key; t += now_ns() - t0; }
            fprintf(fp, "Unsorted SLL,Min,%d,%.2f\n", n, t / REPEATS);

            t = 0; for (int r = 0; r < REPEATS; r++) { double t0 = now_ns();
                SNode *res = us_predecessor(head, fixed); g_sink = res ? res->key : -1;
                t += now_ns() - t0; }
            fprintf(fp, "Unsorted SLL,Predecessor,%d,%.2f\n", n, t / REPEATS);

            t = 0; for (int r = 0; r < REPEATS; r++) { double t0 = now_ns();
                SNode *res = us_successor(head, fixed); g_sink = res ? res->key : -1;
                t += now_ns() - t0; }
            fprintf(fp, "Unsorted SLL,Successor,%d,%.2f\n", n, t / REPEATS);

            t = 0; for (int r = 0; r < REPEATS; r++) { int nk = pool[n + r];
                double t0 = now_ns(); us_insert(&head, nk); t += now_ns() - t0; }
            fprintf(fp, "Unsorted SLL,Insert,%d,%.2f\n", n, t / REPEATS);

            /* worst-case delete = always remove the CURRENT TAIL. Deletion
               cost in a singly linked list depends on DISTANCE FROM HEAD
               (we must walk to find the predecessor), not on whether the
               list happens to be sorted -- so, just like the sorted
               variant below, we deliberately target the tail node. */
            t = 0; for (int r = 0; r < REPEATS; r++) {
                SNode *tgt = sll_find_last(head);       /* untimed: pick the victim */
                double t0 = now_ns(); sll_unlink(&head, tgt); t += now_ns() - t0; }
            fprintf(fp, "Unsorted SLL,Delete,%d,%.2f\n", n, t / REPEATS);

            sll_free_all(head);
        }

        /* ============ 4. SORTED SINGLY LINKED LIST ============ */
        {
            SNode *head = NULL;
            for (int i = 0; i < n; i++) ss_insert(&head, pool[i]);
            SNode *tail_ref = sll_find_last(head);   /* untimed, one-off lookup */
            double t;

            t = 0; for (int r = 0; r < REPEATS; r++) { double t0 = now_ns();
                SNode *res = ss_search(head, ABSENT_KEY); g_sink = res ? res->key : -1;
                t += now_ns() - t0; }
            fprintf(fp, "Sorted SLL,Search,%d,%.2f\n", n, t / REPEATS);

            t = 0; for (int r = 0; r < REPEATS; r++) { double t0 = now_ns();
                SNode *res = sll_find_last(head); g_sink = res->key; t += now_ns() - t0; }
            fprintf(fp, "Sorted SLL,Max,%d,%.2f\n", n, t / REPEATS);

            t = 0; for (int r = 0; r < REPEATS; r++) { double t0 = now_ns();
                g_sink = head->key; t += now_ns() - t0; }
            fprintf(fp, "Sorted SLL,Min,%d,%.2f\n", n, t / REPEATS);

            t = 0; for (int r = 0; r < REPEATS; r++) { double t0 = now_ns();
                SNode *res = sll_find_predecessor(head, tail_ref); g_sink = res ? res->key : -1;
                t += now_ns() - t0; }
            fprintf(fp, "Sorted SLL,Predecessor,%d,%.2f\n", n, t / REPEATS);

            t = 0; for (int r = 0; r < REPEATS; r++) { double t0 = now_ns();
                SNode *res = head->next; g_sink = res ? res->key : -1; t += now_ns() - t0; }
            fprintf(fp, "Sorted SLL,Successor,%d,%.2f\n", n, t / REPEATS);

            /* worst-case insert = new MAXIMUM every time -> must walk the WHOLE list */
            t = 0; for (int r = 0; r < REPEATS; r++) { int nk = POOL_SIZE + 1 + r;
                double t0 = now_ns(); ss_insert(&head, nk); t += now_ns() - t0; }
            fprintf(fp, "Sorted SLL,Insert,%d,%.2f\n", n, t / REPEATS);

            /* worst-case delete = always remove the CURRENT TAIL (finding its
               predecessor needs a full traversal from head)                    */
            t = 0; for (int r = 0; r < REPEATS; r++) {
                SNode *tgt = sll_find_last(head);       /* untimed: pick the victim */
                double t0 = now_ns(); sll_unlink(&head, tgt); t += now_ns() - t0; }
            fprintf(fp, "Sorted SLL,Delete,%d,%.2f\n", n, t / REPEATS);

            sll_free_all(head);
        }

        /* ============ 5. UNSORTED DOUBLY LINKED LIST ============ */
        {
            DNode *head = NULL, *tail = NULL;
            for (int i = 0; i < n; i++) ud_insert(&head, &tail, pool[i]);
            DNode *fixed = head;
            double t;

            t = 0; for (int r = 0; r < REPEATS; r++) { double t0 = now_ns();
                DNode *res = ud_search(head, ABSENT_KEY); g_sink = res ? res->key : -1;
                t += now_ns() - t0; }
            fprintf(fp, "Unsorted DLL,Search,%d,%.2f\n", n, t / REPEATS);

            t = 0; for (int r = 0; r < REPEATS; r++) { double t0 = now_ns();
                DNode *res = ud_max(head); g_sink = res->key; t += now_ns() - t0; }
            fprintf(fp, "Unsorted DLL,Max,%d,%.2f\n", n, t / REPEATS);

            t = 0; for (int r = 0; r < REPEATS; r++) { double t0 = now_ns();
                DNode *res = ud_min(head); g_sink = res->key; t += now_ns() - t0; }
            fprintf(fp, "Unsorted DLL,Min,%d,%.2f\n", n, t / REPEATS);

            t = 0; for (int r = 0; r < REPEATS; r++) { double t0 = now_ns();
                DNode *res = ud_predecessor(head, fixed); g_sink = res ? res->key : -1;
                t += now_ns() - t0; }
            fprintf(fp, "Unsorted DLL,Predecessor,%d,%.2f\n", n, t / REPEATS);

            t = 0; for (int r = 0; r < REPEATS; r++) { double t0 = now_ns();
                DNode *res = ud_successor(head, fixed); g_sink = res ? res->key : -1;
                t += now_ns() - t0; }
            fprintf(fp, "Unsorted DLL,Successor,%d,%.2f\n", n, t / REPEATS);

            t = 0; for (int r = 0; r < REPEATS; r++) { int nk = pool[n + r];
                double t0 = now_ns(); ud_insert(&head, &tail, nk); t += now_ns() - t0; }
            fprintf(fp, "Unsorted DLL,Insert,%d,%.2f\n", n, t / REPEATS);

            t = 0; for (int r = 0; r < REPEATS; r++) {
                double t0 = now_ns(); dll_unlink(&head, &tail, head); t += now_ns() - t0; }
            fprintf(fp, "Unsorted DLL,Delete,%d,%.2f\n", n, t / REPEATS);

            dll_free_all(head);
        }

        /* ============ 6. SORTED DOUBLY LINKED LIST ============ */
        {
            DNode *head = NULL, *tail = NULL;
            for (int i = 0; i < n; i++) sd_insert(&head, &tail, pool[i]);
            double t;

            t = 0; for (int r = 0; r < REPEATS; r++) { double t0 = now_ns();
                DNode *res = sd_search(head, ABSENT_KEY); g_sink = res ? res->key : -1;
                t += now_ns() - t0; }
            fprintf(fp, "Sorted DLL,Search,%d,%.2f\n", n, t / REPEATS);

            t = 0; for (int r = 0; r < REPEATS; r++) { double t0 = now_ns();
                g_sink = tail->key; t += now_ns() - t0; }
            fprintf(fp, "Sorted DLL,Max,%d,%.2f\n", n, t / REPEATS);

            t = 0; for (int r = 0; r < REPEATS; r++) { double t0 = now_ns();
                g_sink = head->key; t += now_ns() - t0; }
            fprintf(fp, "Sorted DLL,Min,%d,%.2f\n", n, t / REPEATS);

            t = 0; for (int r = 0; r < REPEATS; r++) { double t0 = now_ns();
                DNode *res = tail->prev; g_sink = res ? res->key : -1; t += now_ns() - t0; }
            fprintf(fp, "Sorted DLL,Predecessor,%d,%.2f\n", n, t / REPEATS);

            t = 0; for (int r = 0; r < REPEATS; r++) { double t0 = now_ns();
                DNode *res = head->next; g_sink = res ? res->key : -1; t += now_ns() - t0; }
            fprintf(fp, "Sorted DLL,Successor,%d,%.2f\n", n, t / REPEATS);

            /* worst-case insert = new MAXIMUM every time -> walk from head to tail */
            t = 0; for (int r = 0; r < REPEATS; r++) { int nk = POOL_SIZE + 1 + r;
                double t0 = now_ns(); sd_insert(&head, &tail, nk); t += now_ns() - t0; }
            fprintf(fp, "Sorted DLL,Insert,%d,%.2f\n", n, t / REPEATS);

            t = 0; for (int r = 0; r < REPEATS; r++) {
                double t0 = now_ns(); dll_unlink(&head, &tail, head); t += now_ns() - t0; }
            fprintf(fp, "Sorted DLL,Delete,%d,%.2f\n", n, t / REPEATS);

            dll_free_all(head);
        }
    }

    fclose(fp);
    free(pool);
    printf("Done. Results written to q1_timings.csv\n");
    return 0;
}
