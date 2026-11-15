#ifndef REPORT_CLASSES_H
#define REPORT_CLASSES_H

#include <string>
#include <iostream>
#include "common.h"

class Report {
protected:
    std::string title;
    int locationId;

public:
    Report(const std::string &t, int loc);
    virtual ~Report();

    virtual std::string getText() const = 0;

    void print() const;
    void saveToFile(const std::string &path) const;
    std::string getTitle() const;
};

class RiskReport : public Report {
private:
    double score;
    RiskLevel level;

public:
    RiskReport(int loc, double s, RiskLevel l);
    std::string getText() const;
};

class AlertReport : public Report {
private:
    int alertId;
    RiskLevel level;
    double confidence;

public:
    AlertReport(int loc, int id, RiskLevel l, double conf);
    std::string getText() const;
};

std::ostream &operator<<(std::ostream &out, const Report &r);

#endif
