#ifndef DATA_ACQUISITION_H
#define DATA_ACQUISITION_H
#include "common.h"

#define MAX_WATER_LEVEL_M 20.0

typedef struct {
    SensorReading *data;
    int capacity;
    int head;
    int count;
} CircularBuffer;

CircularBuffer *cb_create(int cap);
Status cb_push(CircularBuffer *cb, SensorReading r);
int cb_get_last(CircularBuffer *cb, int n, SensorReading *out);
void cb_free(CircularBuffer *cb);

typedef struct QueueNode {
    SensorReading reading;
    struct QueueNode *next;
} QueueNode;

typedef struct {
    QueueNode *front;
    QueueNode *rear;
    int size;
} ReadingQueue;

ReadingQueue *rq_create(void);
Status rq_enqueue(ReadingQueue *q, SensorReading r);
Status rq_dequeue(ReadingQueue *q, SensorReading *out);
int rq_is_empty(ReadingQueue *q);
void rq_free(ReadingQueue *q);

Status validate_reading(const SensorReading *r);

SensorReading sim_generate(SensorType t, int loc, int scenario, int step);
#endif