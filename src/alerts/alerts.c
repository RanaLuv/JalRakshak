#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "alerts.h"

static int valid_hazard(HazardType h) {
    return h == HAZARD_FLOOD || h == HAZARD_LANDSLIDE;
}

static int bucket_for(int key, int buckets) {
    unsigned int u = (unsigned int)key;
    return (int)(u % (unsigned int)buckets);
}


int make_report_key(int loc, HazardType h, long ts, int window_sec) {
    uint64_t x;
    long win;
    if (window_sec <= 0) window_sec = ALERT_DEFAULT_WINDOW_SEC;
    win = ts / window_sec;
    x = (unsigned long)(unsigned int)loc;
    x = x * 2654435761UL + (unsigned long)(unsigned int)h;
    x = x * 2654435761UL + (unsigned long)win;
    return (int)(x ^ (x >> 32));
}

ReportTable *rt_create(int buckets) {
    ReportTable *t;
    int i;
    if (buckets <= 0) return NULL;
    t = (ReportTable *)malloc(sizeof(*t));
    if (!t) return NULL;
    t->table = (ReportNode **)calloc((size_t)buckets, sizeof(ReportNode *));
    if (!t->table) { free(t); return NULL; }
    t->buckets = buckets;
    t->window_sec = ALERT_DEFAULT_WINDOW_SEC;
    t->count = 0;
    for (i = 0; i < buckets; ++i) t->table[i] = NULL;
    return t;
}

void rt_free(ReportTable *t) {
    int i;
    if (!t) return;
    for (i = 0; i < t->buckets; ++i) {
        ReportNode *cur = t->table[i];
        while (cur) {
            ReportNode *next = cur->next;
            free(cur);
            cur = next;
        }
    }
    free(t->table);
    free(t);
}

int rt_search(ReportTable *t, int key) {
    ReportNode *cur;
    int b;
    if (!t || !t->table) return 0;
    b = bucket_for(key, t->buckets);
    for (cur = t->table[b]; cur; cur = cur->next)
        if (cur->key == key) return 1;
    return 0;
}

int rt_insert(ReportTable *t, CitizenReport r, int window_sec) {
    int key, b;
    long win;
    ReportNode *cur, *node;
    if (!t || !t->table || r.location_id < 0 ||
        !valid_hazard(r.hazard) || r.severity < 1 || r.severity > 5 ||
        r.timestamp < 0) return 0;
    if (window_sec <= 0) window_sec = ALERT_DEFAULT_WINDOW_SEC;
    win = r.timestamp / window_sec;
    key = make_report_key(r.location_id, r.hazard, r.timestamp, window_sec);
    b = bucket_for(key, t->buckets);

    /* Compare full identity, not only the hash, to avoid hash-collision false positives. */
    for (cur = t->table[b]; cur; cur = cur->next) {
        if (cur->key == key && cur->report.location_id == r.location_id &&
            cur->report.hazard == r.hazard && cur->time_window == win)
            return 0;
    }
    node = (ReportNode *)malloc(sizeof(*node));
    if (!node) return 0;
    node->key = key;
    node->report = r;
    node->time_window = win;
    node->next = t->table[b];
    t->table[b] = node;
    t->count++;
    t->window_sec = window_sec;
    return 1;
}

Status process_report(ReportTable *table, const CitizenReport *report) {
    int window;
    if (!table || !report) return STATUS_INVALID;
    if (report->location_id < 0 || !valid_hazard(report->hazard) ||
        report->severity < 1 || report->severity > 5 || report->timestamp < 0)
        return STATUS_INVALID;
    window = table->window_sec > 0 ? table->window_sec : ALERT_DEFAULT_WINDOW_SEC;
    if (!rt_insert(table, *report, window)) {
        /* Distinguish duplicate from allocation failure / invalid input. */
        int key = make_report_key(report->location_id, report->hazard,
                                  report->timestamp, window);
        if (rt_search(table, key)) return STATUS_DUPLICATE;
        return STATUS_ERROR;
    }
    return STATUS_OK;
}

int verified_count(ReportTable *table, int loc, HazardType hazard) {
    int i, count = 0;
    ReportNode *cur;
    if (!table || !valid_hazard(hazard)) return 0;
    for (i = 0; i < table->buckets; ++i)
        for (cur = table->table[i]; cur; cur = cur->next)
            if (cur->report.location_id == loc && cur->report.hazard == hazard)
                count++;
    return count;
}

double compute_confidence(int count) {
    double confidence;
    if (count < 0) count = 0;
    confidence = 0.4 + 0.2 * (double)count;
    if (confidence > 1.0) confidence = 1.0;
    return confidence;
}

AlertHeap *ah_create(int cap) {
    AlertHeap *h;
    if (cap <= 0) return NULL;
    h = (AlertHeap *)malloc(sizeof(*h));
    if (!h) return NULL;
    h->alerts = (Alert *)malloc((size_t)cap * sizeof(Alert));
    if (!h->alerts) { free(h); return NULL; }
    h->size = 0;
    h->capacity = cap;
    return h;
}

void ah_free(AlertHeap *h) {
    if (!h) return;
    free(h->alerts);
    free(h);
}

int ah_push(AlertHeap *h, Alert a) {
    int i;
    if (!h || !h->alerts) return 0;
    if (h->size >= h->capacity) return 0;
    i = h->size++;
    h->alerts[i] = a;
    while (i > 0) {
        int p = (i - 1) / 2;
        Alert tmp;
        if (h->alerts[p].priority >= h->alerts[i].priority) break;
        tmp = h->alerts[p]; h->alerts[p] = h->alerts[i]; h->alerts[i] = tmp;
        i = p;
    }
    return 1;
}

int ah_pop_max(AlertHeap *h, Alert *out) {
    int i = 0;
    if (!h || !out || h->size <= 0) return 0;
    *out = h->alerts[0];
    h->size--;
    if (h->size == 0) return 1;
    h->alerts[0] = h->alerts[h->size];
    for (;;) {
        int left = 2*i + 1, right = 2*i + 2, largest = i;
        Alert tmp;
        if (left < h->size && h->alerts[left].priority > h->alerts[largest].priority)
            largest = left;
        if (right < h->size && h->alerts[right].priority > h->alerts[largest].priority)
            largest = right;
        if (largest == i) break;
        tmp = h->alerts[i]; h->alerts[i] = h->alerts[largest]; h->alerts[largest] = tmp;
        i = largest;
    }
    return 1;
}

int ah_is_empty(AlertHeap *h) { return (!h || h->size == 0); }

int compute_priority(RiskLevel level, double confidence) {
    if (confidence < 0.0) confidence = 0.0;
    if (confidence > 1.0) confidence = 1.0;
    return (int)level * 100 + (int)(confidence * 100.0 + 0.5);
}

Status create_alert(AlertHeap *heap, int loc, const RiskResult *risk,
                    double confidence, long timestamp) {
    static int next_alert_id = 1;
    Alert a;
    if (!heap || !risk || loc < 0 || timestamp < 0 ||
        risk->level < RISK_NORMAL || risk->level > RISK_CRITICAL)
        return STATUS_INVALID;
    if (confidence < 0.0) confidence = 0.0;
    if (confidence > 1.0) confidence = 1.0;
    a.alert_id = next_alert_id++;
    a.location_id = loc;
    a.level = risk->level;
    a.confidence = confidence;
    a.priority = compute_priority(risk->level, confidence);
    a.timestamp = timestamp;
    return ah_push(heap, a) ? STATUS_OK : STATUS_FULL;
}

AlertHistory *history_create(void) {
    return (AlertHistory *)calloc(1, sizeof(AlertHistory));
}

void history_free(AlertHistory *history) {
    AlertHistoryNode *cur, *next;
    if (!history) return;
    cur = history->head;
    while (cur) { next = cur->next; free(cur); cur = next; }
    free(history);
}

Status history_add(AlertHistory *history, const Alert *alert) {
    AlertHistoryNode *node;
    if (!history || !alert) return STATUS_INVALID;
    node = (AlertHistoryNode *)malloc(sizeof(*node));
    if (!node) return STATUS_ERROR;
    node->alert = *alert;
    node->next = NULL;
    if (history->tail) history->tail->next = node;
    else history->head = node;
    history->tail = node;
    history->count++;
    return STATUS_OK;
}

Status history_save_to(const AlertHistory *history, const char *path) {
    FILE *fp;
    const AlertHistoryNode *cur;
    if (!history || !path || !*path) return STATUS_INVALID;
    fp = fopen(path, "w");
    if (!fp) return STATUS_ERROR;
    for (cur = history->head; cur; cur = cur->next) {
        if (fprintf(fp, "%d,%d,%d,%.6f,%d,%ld\n",
                    cur->alert.alert_id, cur->alert.location_id,
                    (int)cur->alert.level, cur->alert.confidence,
                    cur->alert.priority, cur->alert.timestamp) < 0) {
            fclose(fp); return STATUS_ERROR;
        }
    }
    if (fclose(fp) != 0) return STATUS_ERROR;
    return STATUS_OK;
}

Status history_load_from(AlertHistory *history, const char *path) {
    FILE *fp;
    char line[256];
    if (!history || !path || !*path) return STATUS_INVALID;
    fp = fopen(path, "r");
    if (!fp) return STATUS_ERROR;
    while (fgets(line, sizeof(line), fp)) {
        Alert a;
        int level;
        if (sscanf(line, "%d,%d,%d,%lf,%d,%ld",
                   &a.alert_id, &a.location_id, &level, &a.confidence,
                   &a.priority, &a.timestamp) != 6) {
            fclose(fp); return STATUS_INVALID;
        }
        if (level < RISK_NORMAL || level > RISK_CRITICAL ||
            a.confidence < 0.0 || a.confidence > 1.0) {
            fclose(fp); return STATUS_INVALID;
        }
        a.level = (RiskLevel)level;
        if (history_add(history, &a) != STATUS_OK) {
            fclose(fp); return STATUS_ERROR;
        }
    }
    if (ferror(fp)) { fclose(fp); return STATUS_ERROR; }
    fclose(fp);
    return STATUS_OK;
}

void build_message(const Alert *alert, char *out, int size) {
    if (!out || size <= 0) return;
    if (!alert) { snprintf(out, (size_t)size, "JALRAKSHAK: Invalid alert."); return; }
    switch (alert->level) {
        case RISK_CRITICAL:
            snprintf(out, (size_t)size,
                "[JALRAKSHAK] CRITICAL FLOOD/LANDSLIDE ALERT at location %d. "
                "Confidence: %.0f%%. Evacuate immediately using the recommended safe route.",
                alert->location_id, alert->confidence * 100.0);
            break;
        case RISK_WARNING:
            snprintf(out, (size_t)size,
                "[JALRAKSHAK] WARNING at location %d. Confidence: %.0f%%. "
                "Prepare to evacuate and follow official instructions.",
                alert->location_id, alert->confidence * 100.0);
            break;
        case RISK_WATCH:
            snprintf(out, (size_t)size,
                "[JALRAKSHAK] WATCH at location %d. Confidence: %.0f%%. "
                "Stay alert and monitor updates.",
                alert->location_id, alert->confidence * 100.0);
            break;
        default:
            snprintf(out, (size_t)size,
                "[JALRAKSHAK] Status NORMAL at location %d. Confidence: %.0f%%.",
                alert->location_id, alert->confidence * 100.0);
            break;
    }
}
