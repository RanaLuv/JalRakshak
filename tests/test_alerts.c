#include <stdio.h>
#include "common.h"
#include "alerts.h"

int main(void)
{
    ReportTable *table;
    AlertHeap *heap;
    CitizenReport a = {1, 5, HAZARD_FLOOD, 3, 1000};
    CitizenReport b = {2, 5, HAZARD_FLOOD, 3, 1010};
    Alert warning;
    Alert critical;
    Alert next;

    table = rt_create(16);

    if (table == NULL) {
        printf("Couldn't create report table\n");
        return 1;
    }

    printf("1st report added: %d\n", rt_insert(table, a, 60));
    printf("Similar report added: %d\n", rt_insert(table, b, 60));

    heap = ah_create(10);

    if (heap == NULL) {
        printf("couldn't create alert heap\n");
        rt_free(table);
        return 1;
    }

    warning.alert_id = 1;
    warning.location_id = 5;
    warning.level = RISK_WARNING;
    warning.confidence = 0.7;
    warning.priority = compute_priority(warning.level, warning.confidence);
    warning.timestamp = 1000;

    critical.alert_id = 2;
    critical.location_id = 8;
    critical.level = RISK_CRITICAL;
    critical.confidence = 0.9;
    critical.priority = compute_priority(critical.level, critical.confidence);
    critical.timestamp = 1010;

    ah_push(heap, warning);
    ah_push(heap, critical);

    printf("alerts in priority order:\n");
    while (!ah_is_empty(heap)) {
        ah_pop_max(heap, &next);
        printf("alert %d, priority %d\n", next.alert_id, next.priority);
    }

    ah_free(heap);
    rt_free(table);

    return 0;
}
