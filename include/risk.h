#ifndef RISK_H
#define RISK_H

#include "common.h"
#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

// --- TEMPORARY FIX: Jab tak Luv update nahi karta ---
#ifndef RISK_RESULT_DEFINED
#define RISK_RESULT_DEFINED
typedef struct {
    double score;
    RiskLevel level;
    int top_factor;
    int anomaly;
} RiskResult;
#endif
// ----------------------------------------------------

// Config read karne ke liye structs
typedef struct {
    double rain;
    double trend;
    double susceptibility;
    double reports;
} RiskWeights;

typedef struct {
    double spike_limit;
    double rain_max;
} Thresholds;

// Locations ko rank/sort karne ke liye struct
typedef struct {
    int location_id;
    RiskResult result;
} LocationRisk;

// --- Phase 2 ke saare function prototypes ---
Status load_config(const char *path, RiskWeights *w, Thresholds *t);
RiskResult compute_location_risk(int loc, const double *rain, int nr, const double *water, int nw, int reports);
int detect_spike(const double *v, int n, double limit);
RiskResult landslide_risk(double rain_avg, double slope, int reports);
void sort_by_risk(LocationRisk *arr, int n);
void explain_risk(const RiskResult *r);
const char *top_factor_name(int f);

#ifdef __cplusplus
}
#endif

#endif