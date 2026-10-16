#ifndef RISK_H
#define RISK_H

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    double w1, w2, w3, w4;
} RiskWeights;

typedef struct {
    double watch, warning, critical;
} Thresholds;

// C functions declaration
double normalize(double v, double lo, double hi);
double compute_risk(double rain, double trend, double susceptibility, double reports, const RiskWeights *w);
double water_level_trend(const double *vals, int n, int window);
RiskLevel classify_risk(double score, const Thresholds *t);
int detect_spike(const double *vals, int n, double limit);
void explain_risk(double rain, double trend, double sus, double rep, const RiskWeights *w);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

class RiskCalculator {
private:
    RiskWeights weights;
    Thresholds thresholds;

public:
    RiskCalculator();
    RiskCalculator(RiskWeights w, Thresholds t);
    double calculate(double rain, double trend, double sus, double rep);
    RiskLevel getLevel(double score);
};
#endif

#endif