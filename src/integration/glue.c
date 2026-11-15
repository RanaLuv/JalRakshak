#include <stdio.h>
#include "common.h"
#include "glue.h"
#include "data_acquisition.h"
#include "risk.h"
#include "map.h"
#include "routing.h"
#include "alerts.h"

#define KEEP_READINGS 20

static ReadingQueue *queue = NULL;
static CircularBuffer *rain_buffer = NULL;
static CircularBuffer *water_buffer = NULL;
static ReportTable *report_table = NULL;
static AlertHeap *alert_heap = NULL;
static Graph *road_map = NULL;

int glue_start(void) {
    road_map = load_map("data/map.txt");
    if (road_map == NULL) {
        return -1;
    }
    report_table = rt_create(16);
    alert_heap = ah_create(20);
    if (report_table == NULL || alert_heap == NULL) {
        return -1;
    }
    return 0;
}

void glue_end(void) {
    if (road_map != NULL) {
        graph_free(road_map);
    }
    if (report_table != NULL) {
        rt_free(report_table);
    }
    if (queue != NULL) {
        rq_free(queue);
    }
    if (rain_buffer != NULL) {
        cb_free(rain_buffer);
    }
    if (water_buffer != NULL) {
        cb_free(water_buffer);
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
    sort_by_risk(list, n);
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
    if (ah_pop_max(alert_heap, a) != STATUS_OK) {
        return 0;
    }
    history_add(a);
    return 1;
}

void glue_alert_text(const Alert *a, char *out, int size) {
    build_message(a, out, size);
}

int glue_find_place(const char *name) {
    return find_node(road_map, name);
}

const char *glue_place_name(int id) {
    if (id < 0 || id >= road_map->n) {
        return "unknown";
    }
    return road_map->name[id];
}

void glue_set_road(const char *from, const char *to, RoadStatus status) {
    int u;
    int v;

    u = find_node(road_map, from);
    v = find_node(road_map, to);
    if (u < 0 || v < 0) {
        printf("Road %s - %s was not found in the map.\n", from, to);
        return;
    }
    graph_set_road(road_map, u, v, status);
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

void glue_save_history(const char *path) {
    history_save(path);
}

void glue_load_history(const char *path) {
    history_load(path);
}
