#include "risk.h"
#include <stdio.h>
#include <string.h>

// Global variables
static RiskWeights g_weights = {40.0, 30.0, 20.0, 10.0};
static Thresholds g_thresholds = {1.5, 150.0};

// 1. Config Loader
Status load_config(const char *path, RiskWeights *w, Thresholds *t) {
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        printf("Error: Could not open config file: %s\n", path);
        return STATUS_ERROR;
    }

    char key[50];
    double value;

    while (fscanf(file, "%49s %lf", key, &value) == 2) {
        if (strcmp(key, "RAIN_WEIGHT") == 0) w->rain = value;
        else if (strcmp(key, "TREND_WEIGHT") == 0) w->trend = value;
        else if (strcmp(key, "SUSC_WEIGHT") == 0) w->susceptibility = value;
        else if (strcmp(key, "REPORT_WEIGHT") == 0) w->reports = value;
        else if (strcmp(key, "SPIKE_LIMIT") == 0) t->spike_limit = value;
        else if (strcmp(key, "RAIN_MAX") == 0) t->rain_max = value;
    }

    fclose(file);
    g_weights = *w;
    g_thresholds = *t;
    return STATUS_OK;
}

// 2. Anomaly Detection (Sliding Window)
int detect_spike(const double *v, int n, double limit) {
    if (n < 2) return 0;
    for (int i = 1; i < n; i++) {
        if ((v[i] - v[i-1]) >= limit) return 1; 
    }
    return 0;
}

// 3. Main Risk Calculation
RiskResult compute_location_risk(int loc, const double *rain, int nr, const double *water, int nw, int reports) {
    RiskResult result = {0};
    
    double rain_avg = 0;
    for(int i = 0; i < nr; i++) rain_avg += rain[i];
    if (nr > 0) rain_avg /= nr;
    
    double rain_norm = rain_avg / g_thresholds.rain_max;
    if (rain_norm > 1.0) rain_norm = 1.0;
    
    int has_spike = detect_spike(water, nw, g_thresholds.spike_limit);
    
    double rain_score = rain_norm * g_weights.rain;
    double report_score = (reports > 5 ? 1.0 : reports / 5.0) * g_weights.reports;
    double spike_penalty = has_spike ? g_weights.trend : 0.0; 
    
    result.score = rain_score + report_score + spike_penalty;
    if (result.score > 100.0) result.score = 100.0;
    
    // Yahan Luv wale RISK_ prefixes lagaye hain
    if (result.score < 30) result.level = RISK_NORMAL;
    else if (result.score < 60) result.level = RISK_WATCH;
    else if (result.score < 80) result.level = RISK_WARNING;
    else result.level = RISK_CRITICAL;
    
    result.anomaly = has_spike;
    if (rain_score >= report_score) result.top_factor = 0; 
    else result.top_factor = 3; 
    
    return result;
}

// 4. Landslide Indicator
RiskResult landslide_risk(double rain_avg, double slope, int reports) {
    RiskResult result = {0};
    double score = (rain_avg * 0.4) + (slope * 0.4) + (reports * 5.0);
    if (score > 100.0) score = 100.0;
    
    result.score = score;
    // Yahan Luv wale RISK_ prefixes lagaye hain
    if (score < 40) result.level = RISK_NORMAL;
    else if (score < 70) result.level = RISK_WARNING;
    else result.level = RISK_CRITICAL;
    
    result.top_factor = 2;
    result.anomaly = 0;
    return result;
}

// 5. Custom Sorting (No qsort allowed)
void sort_by_risk(LocationRisk *arr, int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (arr[j].result.score < arr[j+1].result.score) {
                LocationRisk temp = arr[j];
                arr[j] = arr[j+1];
                arr[j+1] = temp;
            }
        }
    }
}

// 6. Explain Logic (Printing)
const char *top_factor_name(int f) {
    switch (f) {
        case 0: return "Rain";
        case 1: return "Trend";
        case 2: return "Location";
        case 3: return "Reports";
        default: return "Unknown";
    }
}

void explain_risk(const RiskResult *r) {
    printf("Risk Score: %.2f%%\n", r->score);
    
    printf("Risk Level: ");
    // Yahan Luv wale RISK_ prefixes lagaye hain
    if (r->level == RISK_NORMAL) printf("NORMAL\n");
    else if (r->level == RISK_WATCH) printf("WATCH\n");
    else if (r->level == RISK_WARNING) printf("WARNING\n");
    else if (r->level == RISK_CRITICAL) printf("CRITICAL\n");

    printf("Top Factor: %s\n", top_factor_name(r->top_factor));
    if (r->anomaly == 1) {
        printf("[!] ALERT: Sudden water level spike detected over the sliding window!\n");
    }
    printf("----------------------------------\n");
}