#ifndef GLUE_H
#define GLUE_H

#include "common.h"

int glue_start(void);
void glue_end(void);
int glue_load_scenario(const char *path, int *invalid);
RiskResult glue_risk(int place, int reports);
void glue_sort(LocationRisk *list, int n);
void glue_explain(const RiskResult *r);
int glue_submit_report(CitizenReport r);
int glue_verified_reports(int place);
void glue_make_alert(int place, RiskResult risk, long time);
void glue_clear_alerts(void);
int glue_next_alert(Alert *a);
void glue_alert_text(const Alert *a, char *out, int size);
int glue_find_place(const char *name);
const char *glue_place_name(int id);
void glue_set_road(const char *from, const char *to, RoadStatus status);
Route glue_route(int place, int mode);
void glue_print_route(const Route *r);
void glue_save_history(const char *path);
void glue_load_history(const char *path);

#endif
