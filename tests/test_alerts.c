#include <stdio.h>
#include <string.h>
#include "alerts.h"

#define CHECK(expr, msg) do { if (!(expr)) { \
    printf("FAIL: %s (line %d)\n", msg, __LINE__); return 1; } } while (0)

int main(void) {
    ReportTable *table = rt_create(17);
    AlertHeap *heap = ah_create(8);
    AlertHistory *history = history_create();
    CitizenReport a = {1, 5, HAZARD_FLOOD, 3, 1000};
    CitizenReport duplicate = {2, 5, HAZARD_FLOOD, 4, 1010};
    CitizenReport other = {3, 5, HAZARD_FLOOD, 2, 1070};
    CitizenReport landslide = {4, 5, HAZARD_LANDSLIDE, 2, 1005};
    RiskResult risk;
    Alert popped;
    char message[ALERT_MESSAGE_SIZE];
    int n;

    CHECK(table && heap && history, "allocation");
    CHECK(process_report(table, &a) == STATUS_OK, "first report accepted");
    CHECK(process_report(table, &duplicate) == STATUS_DUPLICATE, "duplicate rejected");
    CHECK(process_report(table, &other) == STATUS_OK, "new time window accepted");
    CHECK(process_report(table, &landslide) == STATUS_OK, "different hazard accepted");
    n = verified_count(table, 5, HAZARD_FLOOD);
    CHECK(n == 2, "count distinct flood reports");
    CHECK(compute_confidence(3) == 1.0, "confidence capped at 1.0");
    CHECK(compute_confidence(1) > 0.59 && compute_confidence(1) < 0.61,
          "confidence formula");

    risk.score = 92.0;
    risk.level = RISK_CRITICAL;
    risk.top_factor = 1;
    risk.anomaly = 1;
    CHECK(create_alert(heap, 5, &risk, compute_confidence(n), 1200) == STATUS_OK,
          "critical alert created");

    risk.level = RISK_WARNING;
    CHECK(create_alert(heap, 7, &risk, 0.8, 1210) == STATUS_OK,
          "warning alert created");
    CHECK(ah_pop_max(heap, &popped), "pop alert");
    CHECK(popped.level == RISK_CRITICAL, "critical alert first");

    CHECK(history_add(history, &popped) == STATUS_OK, "history add");
    CHECK(history_save_to(history, "test_alert_history.tmp") == STATUS_OK,
          "history save");
    {
        AlertHistory *loaded = history_create();
        CHECK(loaded != NULL, "loaded history allocation");
        CHECK(history_load_from(loaded, "test_alert_history.tmp") == STATUS_OK,
              "history load");
        CHECK(loaded->count == 1, "history record restored");
        history_free(loaded);
    }
    remove("test_alert_history.tmp");

    build_message(&popped, message, sizeof(message));
    CHECK(strstr(message, "CRITICAL") != NULL, "critical message generated");
    printf("All Role 5 tests passed. Verified flood reports: %d\n", n);

    history_free(history);
    ah_free(heap);
    rt_free(table);
    return 0;
}
