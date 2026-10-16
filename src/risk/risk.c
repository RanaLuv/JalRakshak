#include <stdio.h>
#include "risk.h"

double normalize(double v, double lo, double hi) {
    double ans = (v - lo) / (hi - lo);
    if (ans < 0.0) {
        ans = 0.0;
    }
    if (ans > 1.0) {
        ans = 1.0;
    }
    return ans;

}
// OBSERVATION FROM PREV READINGS
double water_level_trend(const double *vals, int n, int window) {
    int start = n - window;
    double rise = vals[n - 1] - vals[start];
    return rise / (window - 1);
}

// CALCULATING RISK SCORE
double compute_risk(double rain, double trend, double susceptibility, double reports, const RiskWeights *w) {
    double total = (w->w1 * rain) + (w->w2 * trend) + (w->w3 * susceptibility) + (w->w4 * reports);
    return total * 100.0;
}

// TELLING LEVEL
RiskLevel classify_risk(double score, const Thresholds *t) {
    if (score >= t->critical) {
        return RISK_CRITICAL;
    }
    if (score >= t->warning) {
        return RISK_WARNING;
    }
    if (score >= t->watch) {
        return RISK_WATCH;
    }
    return RISK_NORMAL;
}

// CHECKING SUDDEN INCREASE
int detect_spike(const double *vals, int n, double limit) {
    for (int i = 1; i < n; i++) {
        if ((vals[i] - vals[i - 1]) > limit) {
            return 1;
        }
    }
    return 0;
}

// CAUSE OF RISK
void explain_risk(double rain, double trend, double sus, double rep, const RiskWeights *w) {
    double c_rain = w->w1 * rain;
    double c_trend = w->w2 * trend;
    double c_sus = w->w3 * sus;
    double c_rep = w->w4 * rep;

    printf("\n--- Risk Details ---\n");
    printf("Rainfall contribution: %.2f\n", c_rain * 100.0);
    printf("Water trend contribution: %.2f\n", c_trend * 100.0);
    printf("Susceptibility contribution: %.2f\n", c_sus * 100.0);
    printf("Reports contribution: %.2f\n", c_rep * 100.0);

    if (c_rain >= c_trend && c_rain >= c_sus && c_rain >= c_rep) {
        printf("Main reason: Heavy Rainfall\n");
    } else if (c_trend >= c_rain && c_trend >= c_sus && c_trend >= c_rep) {
        printf("Main reason: Fast Water Rise\n");
    } else if (c_sus >= c_rain && c_sus >= c_trend && c_sus >= c_rep) {
        printf("Main reason: Area Vulnerability\n");
    } else {
        printf("Main reason: Citizen Reports\n");
    }
}