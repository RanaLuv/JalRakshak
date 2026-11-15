#include <stdio.h>
#include "data_acquisition.h"

int main(void) {
    int i;

    /* 1. Buffer of 5: push 8, only the last 5 stay */
    CircularBuffer *cb = cb_create(5);
    for (i = 0; i < 8; i++)
        cb_push(cb, sim_generate(SENSOR_RAIN, 1, 0, i));
    SensorReading out[5];
    printf("kept %d readings\n", cb_get_last(cb, 5, out));
    cb_print(cb);
    cb_free(cb);

    /* 2. Invalid reading rejected */
    SensorReading bad = sim_generate(SENSOR_RAIN, 1, 2, 0);
    printf("invalid reading -> status %d (should be %d)\n",
           validate_reading(&bad), STATUS_INVALID);

    /* 3. Queue order: first in, first out */
    ReadingQueue *q0 = rq_create();
    for (i = 0; i < 3; i++) rq_enqueue(q0, sim_generate(SENSOR_RAIN, 1, 1, i));
    SensorReading r;
    while (rq_dequeue(q0, &r) == STATUS_OK) printf("out: %.1f\n", r.value);
    rq_free(q0);

    /* 4. Flood file: 13 valid lines, 1 rejected */
    generate_scenario_file("data/readings_test.txt", 1, 6);
    FILE *f = fopen("data/readings_test.txt", "a");
    fprintf(f, "10 0 5.00 1700000999 1\n");   /* valid line */
    fprintf(f, "10 0 -1.00 1700001000 1\n");  /* bad line: negative rain */
    fclose(f);

    int invalid = 0;
    ReadingQueue *q = rq_create();
    int ok = load_readings("data/readings_test.txt", q, &invalid);
    printf("read %d valid, %d rejected\n", ok, invalid);

    /* 5. Ingest into rain and water buffers (size 5) */
    CircularBuffer *rain = cb_create(5), *water = cb_create(5);
    ingest_all(q, rain, water);
    cb_print(rain);
    cb_print(water);

    /* 6. Recent values as plain doubles */
    double v[3];
    int n = get_recent_values(water, 3, v);
    printf("last %d water values:", n);
    for (i = 0; i < n; i++) printf(" %.2f", v[i]);
    printf("\n");

    /* 7. Missing file */
    printf("missing file -> %d (should be -1)\n",
           load_readings("data/nofile.txt", q, &invalid));

    /* 8. Free everything */
    cb_free(rain); cb_free(water); rq_free(q);
    return 0;
}