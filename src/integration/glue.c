#include <stdio.h>
#include "common.h"
#include "glue.h"
#include "data_acquisition.h"
#include "risk.h"
#include "map.h"
#include "alerts.h"
#include "routing.h"

#define KEEP_READINGS 20

static ReadingQueue *queue = NULL;
static CircularBuffer *rain_buffer = NULL;
static CircularBuffer *water_buffer = NULL;
static ReportTable *report_table = NULL;
static AlertHeap *alert_heap = NULL;
static AlertHistory *alert_history = NULL;
static Graph *road_map = NULL;

int glue_start(void) {
    RiskWeights weights;
    Thresholds limits;

    weights.rain = 40.0;
    weights.trend = 30.0;
    weights.susceptibility = 20.0;
    weights.reports = 10.0;
    limits.spike_limit = 1.5;
    limits.rain_max = 150.0;

    road_map = build_sample_map();
    if (road_map == NULL) {
        return -1;
    }
    report_table = rt_create(16);
    alert_heap = ah_create(20);
    alert_history = history_create();
    if (report_table == NULL || alert_heap == NULL || alert_history == NULL) {
        return -1;
    }
    if (load_config("data/config.txt", &weights, &limits) != STATUS_OK) {
        printf("Warning: could not read data/config.txt. Default values will be used.\n");
    }
    return 0;
}

void glue_end(void) {
    if (road_map != NULL) {
        graph_free(road_map);
        road_map = NULL;
    }
    if (report_table != NULL) {
        rt_free(report_table);
        report_table = NULL;
    }
    if (alert_heap != NULL) {
        ah_free(alert_heap);
        alert_heap = NULL;
    }
    if (alert_history != NULL) {
        history_free(alert_history);
        alert_history = NULL;
    }
    if (queue != NULL) {
        rq_free(queue);
        queue = NULL;
    }
    if (rain_buffer != NULL) {
        cb_free(rain_buffer);
        rain_buffer = NULL;
    }
    if (water_buffer != NULL) {
        cb_free(water_buffer);
        water_buffer = NULL;
    }
}

int glue_load_scenario(const char *path, int *invalid) {
    if (queue != NULL) {
        rq_free(queue);
    }
    if (rain_buffer != NULL) {
        cb_free(rain_buffer);
    }
    if (water_buffer != NULL) {
        cb_free(water_buffer);
    }
    queue = rq_create();
    rain_buffer = cb_create(KEEP_READINGS);
    water_buffer = cb_create(KEEP_READINGS);
    if (load_readings(path, queue, invalid) < 0) {
        return -1;
    }
    return ingest_all(queue, rain_buffer, water_buffer);
}

RiskResult glue_risk(int place, int reports) {
    double rain[KEEP_READINGS];
    double water[KEEP_READINGS];
    int rain_count;
    int water_count;

    rain_count = get_recent_values(rain_buffer, KEEP_READINGS, rain);
    water_count = get_recent_values(water_buffer, KEEP_READINGS, water);
    return compute_location_risk(place, rain, rain_count, water, water_count, reports);
}

void glue_sort(LocationRisk *list, int n) {
    int i;
    LocationRisk temp;

    sort_by_risk(list, n);
    if (n > 1 && list[0].result.score < list[n - 1].result.score) {
        for (i = 0; i < n / 2; i++) {
            temp = list[i];
            list[i] = list[n - 1 - i];
            list[n - 1 - i] = temp;
        }
    }
}

void glue_explain(const RiskResult *r) {
    explain_risk(r);
}

int glue_submit_report(CitizenReport r) {
    return process_report(report_table, &r);
}

int glue_verified_reports(int place) {
    return verified_count(report_table, place, HAZARD_FLOOD);
}

void glue_make_alert(int place, RiskResult risk, long time) {
    double confidence;

    confidence = compute_confidence(glue_verified_reports(place));
    create_alert(alert_heap, place, &risk, confidence, time);
}

void glue_clear_alerts(void) {
    Alert a;

    while (ah_is_empty(alert_heap) == 0) {
        ah_pop_max(alert_heap, &a);
    }
}

int glue_next_alert(Alert *a) {
    if (ah_is_empty(alert_heap) != 0) {
        return 0;
    }
    if (ah_pop_max(alert_heap, a) == 0) {
        return 0;
    }
    history_add(alert_history, a);
    return 1;
}

void glue_alert_text(const Alert *a, char *out, int size) {
    build_message(a, out, size);
}

int glue_history_count(void) {
    if (alert_history == NULL) {
        return 0;
    }
    return alert_history->count;
}

int glue_history_get(int index, Alert *out) {
    AlertHistoryNode *node;
    int i = 0;

    if (alert_history == NULL) {
        return 0;
    }
    node = alert_history->head;
    while (node != NULL) {
        if (i == index) {
            *out = node->alert;
            return 1;
        }
        node = node->next;
        i++;
    }
    return 0;
}

int glue_history_save(const char *path) {
    return history_save_to(alert_history, path);
}

int glue_history_load(const char *path) {
    AlertHistory *fresh;
    int result;

    fresh = history_create();
    if (fresh == NULL) {
        return STATUS_ERROR;
    }
    result = history_load_from(fresh, path);
    if (result != STATUS_OK) {
        history_free(fresh);
        return result;
    }
    history_free(alert_history);
    alert_history = fresh;
    return STATUS_OK;
}

int glue_node_count(void) {
    return road_map->n;
}

int glue_is_shelter(int id) {
    return road_map->is_shelter[id];
}

const char *glue_place_name(int id) {
    if (id < 0 || id >= road_map->n) {
        return "unknown";
    }
    return road_map->name[id];
}

int glue_set_road(int from, int to, RoadStatus status) {
    return graph_set_road(road_map, from, to, status);
}

Route glue_route(int place, int mode) {
    return find_best_route(road_map, place, mode);
}

void glue_print_route(const Route *r) {
    int i;

    for (i = 0; i < r->length; i++) {
        printf("%s", glue_place_name(r->path[i]));
        if (i < r->length - 1) {
            printf(" -> ");
        }
    }
    printf("\n");
}
