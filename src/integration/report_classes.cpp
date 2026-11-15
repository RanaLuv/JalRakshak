#include <fstream>
#include <sstream>
#include <stdexcept>
#include "report_classes.h"

using namespace std;

static string levelText(RiskLevel level) {
    switch (level) {
        case RISK_NORMAL:   return "NORMAL";
        case RISK_WATCH:    return "WATCH";
        case RISK_WARNING:  return "WARNING";
        case RISK_CRITICAL: return "CRITICAL";
        default:            return "UNKNOWN";
    }
}

Report::Report(const string &t, int loc) {
    title = t;
    locationId = loc;
}

Report::~Report() {
}

void Report::print() const {
    cout << "--- " << title << " ---" << endl;
    cout << getText() << endl;
}

void Report::saveToFile(const string &path) const {
    ofstream file(path.c_str(), ios::app);
    if (!file) {
        throw runtime_error("Cannot open file: " + path);
    }
    file << "--- " << title << " ---" << endl;
    file << getText() << endl;
}

string Report::getTitle() const {
    return title;
}

RiskReport::RiskReport(int loc, double s, RiskLevel l) : Report("Risk Report", loc) {
    score = s;
    level = l;
}

string RiskReport::getText() const {
    ostringstream text;
    text << "Location: " << locationId << "\n"
         << "Risk score: " << score << " out of 100\n"
         << "Risk level: " << levelText(level);
    return text.str();
}

AlertReport::AlertReport(int loc, int id, RiskLevel l, double conf) : Report("Alert Report", loc) {
    alertId = id;
    level = l;
    confidence = conf;
}

string AlertReport::getText() const {
    ostringstream text;
    text << "Alert number: " << alertId << "\n"
         << "Location: " << locationId << "\n"
         << "Level: " << levelText(level) << "\n"
         << "Confidence: " << confidence * 100 << "%\n"
         << "Message: Flood risk is high. Please check the safe route.";
    return text.str();
}

ostream &operator<<(ostream &out, const Report &r) {
    out << "[" << r.getTitle() << "]\n" << r.getText();
    return out;
}
