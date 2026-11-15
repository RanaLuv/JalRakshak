#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

/* ---------- NEW: final phase functions ---------- */

/* Reads file (sensor_id type value timestamp location_id per line).
   Valid lines go into the queue. Returns valid count, -1 if file missing. */
int load_readings(const char *path, ReadingQueue *q, int *invalid) {
    FILE *f = fopen(path, "r");
    char line[256];
    int valid = 0;
    if (!f || !q || !invalid) { if (f) fclose(f); return -1; }
    *invalid = 0;
    while (fgets(line, sizeof line, f)) {
        SensorReading r;
        int type;
        if (line[strspn(line, " \t\r\n")] == '\0') continue;
        if (sscanf(line, "%d %d %lf %ld %d", &r.sensor_id, &type,
                   &r.value, &r.timestamp, &r.location_id) != 5) {
            (*invalid)++;
            continue;
        }
        r.type = (SensorType)type;
        if (validate_reading(&r) != STATUS_OK) { (*invalid)++; continue; }
        if (rq_enqueue(q, r) == STATUS_OK) valid++;
    }
    fclose(f);
    return valid;
}

/* Empties the queue into the rain and water buffers. Returns count stored. */
int ingest_all(ReadingQueue *q, CircularBuffer *rain, CircularBuffer *water) {
    SensorReading r;
    int n = 0;
    if (!q || !rain || !water) return 0;
    while (rq_dequeue(q, &r) == STATUS_OK) {
        if (validate_reading(&r) != STATUS_OK) continue;
        if (r.type == SENSOR_RAIN) cb_push(rain, r);
        else cb_push(water, r);
        n++;
    }
    return n;
}

/* Copies the last n values as plain doubles (oldest first). Returns count. */
int get_recent_values(CircularBuffer *cb, int n, double *out) {
    SensorReading *tmp;
    int got, i;
    if (!cb || !out || n <= 0) return 0;
    tmp = malloc(n * sizeof(SensorReading));
    if (!tmp) return 0;
    got = cb_get_last(cb, n, tmp);
    for (i = 0; i < got; i++) out[i] = tmp[i].value;
    free(tmp);
    return got;
}

/* Latest value in the buffer, 0.0 if empty. */
double cb_latest(CircularBuffer *cb) {
    if (!cb || cb->count == 0) return 0.0;
    return cb->data[(cb->head - 1 + cb->capacity) % cb->capacity].value;
}

/* Prints all stored values with count, min, max and latest. */
void cb_print(CircularBuffer *cb) {
    int i, start;
    double mn, mx;
    if (!cb || cb->count == 0) { printf("Buffer empty\n"); return; }
    start = (cb->head - cb->count + cb->capacity) % cb->capacity;
    mn = mx = cb->data[start].value;
    printf("Buffer (%d/%d): ", cb->count, cb->capacity);
    for (i = 0; i < cb->count; i++) {
        double v = cb->data[(start + i) % cb->capacity].value;
        if (v < mn) mn = v;
        if (v > mx) mx = v;
        printf("%.2f ", v);
    }
    printf("\nmin %.2f  max %.2f  latest %.2f\n", mn, mx, cb_latest(cb));
}

/* Writes a scenario file: 0 normal, 1 warning (rapid rise), 2 critical.
   Returns 0 on success, -1 on failure. */
int generate_scenario_file(const char *path, int scenario, int steps) {
    FILE *f = fopen(path, "w");
    int s, t;
    if (!f || steps <= 0) { if (f) fclose(f); return -1; }
    for (s = 0; s < steps; s++) {
        for (t = 0; t <= 1; t++) {
            SensorReading r = sim_generate((SensorType)t, 1,
                                           scenario == 2 ? 1 : scenario, s);
            if (scenario == 2) {
                r.value = (t == SENSOR_RAIN) ? 60.0 + 15.0 * s
                                             : 8.0 + 2.0 * s;
                if (t == SENSOR_WATER_LEVEL && r.value > 19.5) r.value = 19.5;
            }
            fprintf(f, "%d %d %.2f %ld %d\n", r.sensor_id, (int)r.type,
                    r.value, r.timestamp, r.location_id);
        }
    }
    fclose(f);
    return 0;
}