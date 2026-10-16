#include <stdlib.h>
#include "data_acquisition.h"

#define SIM_START_TIME   1700000000L
#define SIM_STEP_SECONDS 60

CircularBuffer *cb_create(int cap) {
    if (cap <= 0) return NULL;
    CircularBuffer *cb = malloc(sizeof(CircularBuffer));
    if (!cb) return NULL;
    cb->data = malloc(cap * sizeof(SensorReading));
    if (!cb->data) { free(cb); return NULL; }
    cb->capacity = cap;
    cb->head = 0;
    cb->count = 0;
    return cb;
}

Status cb_push(CircularBuffer *cb, SensorReading r) {
    if (!cb) return STATUS_ERROR;
    cb->data[cb->head] = r;
    cb->head = (cb->head + 1) % cb->capacity;
    if (cb->count < cb->capacity) cb->count++;
    return STATUS_OK;
}

int cb_get_last(CircularBuffer *cb, int n, SensorReading *out) {
    if (!cb || !out || n <= 0) return 0;
    int k = (n < cb->count) ? n : cb->count;
    int start = (cb->head - k + cb->capacity) % cb->capacity;
    for (int i = 0; i < k; i++)
        out[i] = cb->data[(start + i) % cb->capacity];
    return k;
}

void cb_free(CircularBuffer *cb) {
    if (!cb) return;
    free(cb->data);
    free(cb);
}

ReadingQueue *rq_create(void) {
    ReadingQueue *q = malloc(sizeof(ReadingQueue));
    if (!q) return NULL;
    q->front = q->rear = NULL;
    q->size = 0;
    return q;
}

Status rq_enqueue(ReadingQueue *q, SensorReading r) {
    if (!q) return STATUS_ERROR;
    QueueNode *node = malloc(sizeof(QueueNode));
    if (!node) return STATUS_ERROR;
    node->reading = r;
    node->next = NULL;
    if (q->rear) q->rear->next = node;
    else q->front = node;
    q->rear = node;
    q->size++;
    return STATUS_OK;
}

Status rq_dequeue(ReadingQueue *q, SensorReading *out) {
    if (!q || !out) return STATUS_ERROR;
    if (!q->front) return STATUS_EMPTY;
    QueueNode *node = q->front;
    *out = node->reading;
    q->front = node->next;
    if (!q->front) q->rear = NULL;
    free(node);
    q->size--;
    return STATUS_OK;
}

int rq_is_empty(ReadingQueue *q) {
    return (!q || q->front == NULL);
}

void rq_free(ReadingQueue *q) {
    if (!q) return;
    QueueNode *cur = q->front;
    while (cur) {
        QueueNode *next = cur->next;
        free(cur);
        cur = next;
    }
    free(q);
}

Status validate_reading(const SensorReading *r) {
    if (!r) return STATUS_ERROR;
    if (r->type != SENSOR_RAIN && r->type != SENSOR_WATER_LEVEL) return STATUS_INVALID;
    if (r->value < 0.0) return STATUS_INVALID;
    if (r->type == SENSOR_WATER_LEVEL && r->value > MAX_WATER_LEVEL_M)
        return STATUS_INVALID;
    if (r->timestamp <= 0) return STATUS_INVALID;
    return STATUS_OK;
}

SensorReading sim_generate(SensorType t, int loc, int scenario, int step) {
    SensorReading r;
    r.sensor_id = loc * 10 + (int)t;
    r.type = t;
    r.location_id = loc;
    r.timestamp = SIM_START_TIME + (long)step * SIM_STEP_SECONDS;

    if (scenario == 1) {
        if (t == SENSOR_RAIN) {
            r.value = 20.0 + 10.0 * step;
        } else {
            r.value = 3.0 + 1.5 * step;
            if (r.value > 19.5) r.value = 19.5;
        }
    } else if (scenario == 2) {
        r.value = -1.0;
    } else {
        if (t == SENSOR_RAIN) r.value = 2.0 + (step % 3);
        else                  r.value = 1.0 + 0.1 * (step % 3);
    }
    return r;
}