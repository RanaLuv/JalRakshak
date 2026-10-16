#include <iostream>
#include "risk.h"

// MEMBER FUNCTIONS
RiskCalculator::RiskCalculator() {
    weights.w1 = 0.35;
    weights.w2 = 0.35;
    weights.w3 = 0.15;
    weights.w4 = 0.15;

    thresholds.watch = 25.0;
    thresholds.warning = 50.0;
    thresholds.critical = 75.0;
}

RiskCalculator::RiskCalculator(RiskWeights w, Thresholds t) {
    weights = w;
    thresholds = t;
}

double RiskCalculator::calculate(double rain, double trend, double sus, double rep) {
    return compute_risk(rain, trend, sus, rep, &weights);
}

RiskLevel RiskCalculator::getLevel(double score) {
    return classify_risk(score, &thresholds);
}

int main() {
    std::cout << "=== Section 1: Data Structures Evaluation in C ===" << std::endl;

    double water_readings[] = {1.0, 1.1, 1.2, 2.0, 3.5, 5.0};
    int n = 6;
    int window = 3;

    // SLIDING WINDOW
    double trend = water_level_trend(water_readings, n, window);
    std::cout << "Sliding Window Trend: " << trend << std::endl;

    // ARRAY SEARCH
    int spike = detect_spike(water_readings, n, 1.0);
    std::cout << "Spike Detected in Readings: " << (spike ? "Yes" : "No") << std::endl;

    double r = normalize(80.0, 0.0, 100.0);
    double tr = normalize(trend, 0.0, 2.0);

    std::cout << "\n=== Section 2: OOPs Evaluation in C++ ===" << std::endl;

    // OBJECT CREATION AND ENCAPSULATION
    RiskCalculator calc;
    double score = calc.calculate(r, tr, 0.6, 0.2);
    RiskLevel lvl = calc.getLevel(score);

    std::cout << "Calculated Score via Object: " << score << std::endl;
    std::cout << "Risk Level: " << lvl << std::endl;

    RiskWeights w = {0.35, 0.35, 0.15, 0.15};
    explain_risk(r, tr, 0.6, 0.2, &w);

    return 0;
}