#include <stdio.h>
#include <stdlib.h>
#include "alerts.h"

ReportTable *rt_create(int buckets)
{
    int i;
    ReportTable *t;

    if (buckets <= 0)
        return NULL;

    t = malloc(sizeof(ReportTable));
    if (t == NULL)
        return NULL;

    t->table = malloc(buckets * sizeof(ReportNode *));
    if (t->table == NULL) {
        free(t);
        return NULL;
    }

    t->buckets = buckets;

    for (i = 0; i < buckets; i++)
        t->table[i] = NULL;

    return t;
}

void rt_free(ReportTable *t)
{
    int i;
    ReportNode *current;
    ReportNode *temp;

    if (t == NULL)
        return;

    for (i = 0; i < t->buckets; i++) {
        current = t->table[i];

        while (current != NULL) {
            temp = current;
            current = current->next;
            free(temp);
        }
    }

    free(t->table);
    free(t);
}

int make_report_key(int loc, HazardType h, long ts, int window_sec)
{
    long time_window;

    if (window_sec <= 0)
        window_sec = 60;

    time_window = ts / window_sec;

    return (int)(loc * 100000L + (int)h * 10000L + time_window);
}

int rt_search(ReportTable *t, int key)
{
    int bucket;
    ReportNode *current;

    if (t == NULL)
        return 0;

    bucket = key % t->buckets;
    if (bucket < 0)
        bucket = -bucket;

    current = t->table[bucket];

    while (current != NULL) {
        if (current->key == key)
            return 1;

        current = current->next;
    }

    return 0;
}

int rt_insert(ReportTable *t, CitizenReport r, int window_sec)
{
    int key;
    int bucket;
    ReportNode *node;

    if (t == NULL)
        return 0;

    key = make_report_key(r.location_id, r.hazard, r.timestamp, window_sec);

    if (rt_search(t, key))
        return 0;

    bucket = key % t->buckets;
    if (bucket < 0)
        bucket = -bucket;

    node = malloc(sizeof(ReportNode));
    if (node == NULL)
        return 0;

    node->key = key;
    node->report = r;
    node->next = t->table[bucket];
    t->table[bucket] = node;

    return 1;
}

int compute_priority(RiskLevel level, double confidence)
{
    if (confidence < 0)
        confidence = 0;
    if (confidence > 1)
        confidence = 1;

    return (int)level * 100 + (int)(confidence * 100);
}

AlertHeap *ah_create(int cap)
{
    AlertHeap *h;

    if (cap <= 0)
        return NULL;

    h = malloc(sizeof(AlertHeap));
    if (h == NULL)
        return NULL;

    h->alerts = malloc(cap * sizeof(Alert));
    if (h->alerts == NULL) {
        free(h);
        return NULL;
    }

    h->size = 0;
    h->capacity = cap;

    return h;
}

void ah_free(AlertHeap *h)
{
    if (h == NULL)
        return;

    free(h->alerts);
    free(h);
}

int ah_push(AlertHeap *h, Alert a)
{
    int i;
    Alert temp;

    if (h == NULL || h->size == h->capacity)
        return 0;

    i = h->size;
    h->alerts[i] = a;
    h->size++;

    while (i > 0 && h->alerts[(i - 1) / 2].priority < h->alerts[i].priority) {
        temp = h->alerts[i];
        h->alerts[i] = h->alerts[(i - 1) / 2];
        h->alerts[(i - 1) / 2] = temp;
        i = (i - 1) / 2;
    }

    return 1;
}

int ah_pop_max(AlertHeap *h, Alert *out)
{
    int i;
    int largest;
    int left;
    int right;
    Alert temp;

    if (h == NULL || h->size == 0 || out == NULL)
        return 0;

    h->alerts[0] = h->alerts[h->size - 1];
    h->size--;

    i = 0;

    while (1) {
        left = 2 * i + 1;
        right = 2 * i + 2;
        largest = i;

        if (left < h->size &&
            h->alerts[left].priority > h->alerts[largest].priority)
            largest = left;

        if (right < h->size &&
            h->alerts[right].priority > h->alerts[largest].priority)
            largest = right;

        if (largest == i)
            break;

        temp = h->alerts[i];
        h->alerts[i] = h->alerts[largest];
        h->alerts[largest] = temp;

        i = largest;
    }

    return 1;
}

int ah_is_empty(AlertHeap *h)
{
    if (h == NULL || h->size == 0)
        return 1;

    return 0;
}
