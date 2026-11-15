#ifndef ALERTS_H
#define ALERTS_H

#include <stddef.h>
#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ALERT_DEFAULT_WINDOW_SEC 60
#define ALERT_DEFAULT_CAPACITY 100
#define ALERT_MESSAGE_SIZE 512

typedef struct ReportNode {
    int key;
    CitizenReport report;
    long time_window;
    struct ReportNode *next;
} ReportNode;

typedef struct {
    int buckets;
    ReportNode **table;
    int window_sec;
    int count;
} ReportTable;

typedef struct {
    Alert *alerts;
    int size;
    int capacity;
} AlertHeap;

typedef struct AlertHistoryNode {
    Alert alert;
    struct AlertHistoryNode *next;
} AlertHistoryNode;

typedef struct {
    AlertHistoryNode *head;
    AlertHistoryNode *tail;
    int count;
} AlertHistory;

/* Report hash table and community-report processing */
ReportTable *rt_create(int buckets);
void rt_free(ReportTable *t);
int make_report_key(int loc, HazardType h, long ts, int window_sec);
int rt_insert(ReportTable *t, CitizenReport r, int window_sec);
int rt_search(ReportTable *t, int key);
Status process_report(ReportTable *table, const CitizenReport *report);
int verified_count(ReportTable *table, int loc, HazardType hazard);
double compute_confidence(int count);

/* Alert max-heap */
AlertHeap *ah_create(int cap);
void ah_free(AlertHeap *h);
int ah_push(AlertHeap *h, Alert a);
int ah_pop_max(AlertHeap *h, Alert *out);
int ah_is_empty(AlertHeap *h);
int compute_priority(RiskLevel level, double confidence);
Status create_alert(AlertHeap *heap, int loc, const RiskResult *risk,
                    double confidence, long timestamp);

/* Handled-alert history */
AlertHistory *history_create(void);
void history_free(AlertHistory *history);
Status history_add(AlertHistory *history, const Alert *alert);
Status history_save_to(const AlertHistory *history, const char *path);
Status history_load_from(AlertHistory *history, const char *path);

/* SMS-style message */
void build_message(const Alert *alert, char *out, int size);

#ifdef __cplusplus
}
#endif

#endif
