#ifndef RISK_CALCULATOR_HPP
#define RISK_CALCULATOR_HPP

// Piche jaakar include folder se risk.h utha rahe hain
#include "../../include/risk.h" 
#include <stdexcept>
#include <vector>
#include <string>
#include <iostream>

// 1. Exception Handling: Custom Exception class
class ConfigException : public std::runtime_error {
public:
    ConfigException(const std::string& msg) : std::runtime_error(msg) {}
};

// 2. Abstract Base Class (Polymorphism)
class Detector {
public:
    virtual ~Detector() {}
    virtual bool detect(const double* values, int n, double limit) = 0; 
};

// 3. Inheritance: Spike Detector
class SpikeDetector : public Detector {
public:
    bool detect(const double* values, int n, double limit) override {
        return detect_spike(values, n, limit) == 1; 
    }
};

// 3. Inheritance: Trend Detector
class TrendDetector : public Detector {
public:
    bool detect(const double* values, int n, double limit) override {
        if (n < 2) return false;
        return (values[n-1] > values[0]); 
    }
};

// 4. Composition & Encapsulation: Risk Calculator
class RiskCalculator {
private:
    RiskWeights weights;
    std::vector<Detector*> detectors;

public:
    RiskCalculator() {
        weights = {40.0, 30.0, 20.0, 10.0}; 
    }

    ~RiskCalculator() {
        for (auto d : detectors) delete d;
    }

    void add_detector(Detector* d) {
        detectors.push_back(d);
    }

    void set_weights(double r, double t, double s, double rep) {
        if ((r + t + s + rep) != 100.0) {
            throw ConfigException("Error: Risk weights must add up to exactly 100!");
        }
        weights.rain = r;
        weights.trend = t;
        weights.susceptibility = s;
        weights.reports = rep;
    }

    RiskResult calculate(int loc, const double* rain, int nr, const double* water, int nw, int reports) {
        return compute_location_risk(loc, rain, nr, water, nw, reports);
    }
};

#endif