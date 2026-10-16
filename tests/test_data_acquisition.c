#include <stdio.h>
#include "data_acquisition.h"

static int fails = 0;
static void check(const char *name, int ok) {
    printf("[%s] %s\n", ok ? "PASS" : "FAIL", name);
    if (!ok) fails++;
}

int main(void) {
    CircularBuffer *cb = cb_create(5);
    for (int i = 0; i < 8; i++) {
        SensorReading r = sim_generate(SENSOR_RAIN, 1, 0, i);
        cb_push(cb, r);
    }
    SensorReading out[5];
    int n = cb_get_last(cb, 5, out);
    printf("kept %d readings\n", n);
    check("buffer keeps 5", n == 5);
    check("oldest kept is step 3", out[0].timestamp == 1700000000L + 3 * 60);
    check("newest kept is step 7", out[4].timestamp == 1700000000L + 7 * 60);
    check("asking for 2 gives 2", cb_get_last(cb, 2, out) == 2);
    cb_free(cb);

    SensorReading ok = sim_generate(SENSOR_RAIN, 1, 0, 0);
    SensorReading neg = sim_generate(SENSOR_RAIN, 1, 2, 0);
    SensorReading high = ok; high.type = SENSOR_WATER_LEVEL; high.value = 25.0;
    SensorReading badts = ok; badts.timestamp = 0;
    check("normal reading valid", validate_reading(&ok) == STATUS_OK);
    check("negative rain rejected", validate_reading(&neg) == STATUS_INVALID);
    check("water above 20 m rejected", validate_reading(&high) == STATUS_INVALID);
    check("bad timestamp rejected", validate_reading(&badts) == STATUS_INVALID);

    SensorReading a = sim_generate(SENSOR_WATER_LEVEL, 1, 1, 0);
    SensorReading b = sim_generate(SENSOR_WATER_LEVEL, 1, 1, 3);
    check("rapid rise grows", b.value > a.value);
    check("rapid rise stays valid", validate_reading(&b) == STATUS_OK);

    ReadingQueue *q = rq_create();
    check("new queue is empty", rq_is_empty(q));
    for (int i = 0; i < 3; i++) rq_enqueue(q, sim_generate(SENSOR_RAIN, 1, 0, i));
    SensorReading x;
    int order_ok = 1;
    for (int i = 0; i < 3; i++) {
        rq_dequeue(q, &x);
        printf("dequeued step %d\n", i);
        if (x.timestamp != 1700000000L + i * 60) order_ok = 0;
    }
    check("queue order FIFO", order_ok);
    check("dequeue on empty gives STATUS_EMPTY", rq_dequeue(q, &x) == STATUS_EMPTY);
    rq_free(q);

    printf(fails ? "\n%d test(s) FAILED\n" : "\nAll tests passed\n", fails);
    return fails;
}