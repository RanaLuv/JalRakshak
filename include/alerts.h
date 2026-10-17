#ifndef ALERTS_H
#define ALERTS_H
#include "common.h"

typedef struct ReportNode {
    int key;
    CitizenReport report;
    struct ReportNode *next;
} ReportNode;

typedef struct {
    int buckets;
    ReportNode **table;
} ReportTable;

typedef struct {
    Alert *alerts;
    int size;
    int capacity;
} AlertHeap;

ReportTable *rt_create(int buckets);
void rt_free(ReportTable *t);
int make_report_key(int loc, HazardType h, long ts, int window_sec);
int rt_insert(ReportTable *t, CitizenReport r, int window_sec);
int rt_search(ReportTable *t, int key);

AlertHeap *ah_create(int cap);
void ah_free(AlertHeap *h);
int ah_push(AlertHeap *h, Alert a);
int ah_pop_max(AlertHeap *h, Alert *out);
int ah_is_empty(AlertHeap *h);

int compute_priority(RiskLevel level, double confidence);

#endif
