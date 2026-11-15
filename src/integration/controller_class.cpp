#include <iostream>
#include <fstream>
#include "controller_class.h"
#include "report_classes.h"
#include "system_controller.h"

using namespace std;

SystemException::SystemException(const string &msg) {
    message = msg;
}

const char *SystemException::what() const noexcept {
    return message.c_str();
}

SystemController::SystemController() {
    projectName = "JalRakshak";
    sessionCount = 0;
}

SystemController::SystemController(const string &name) {
    projectName = name;
    sessionCount = 0;
}

SystemController::~SystemController() {
    cout << "Controller for " << projectName << " closed." << endl;
}

void SystemController::checkFileExists(const string &path) {
    ifstream file(path.c_str());
    if (!file) {
        throw SystemException("Missing file: " + path);
    }
}

void SystemController::checkDataFiles() {
    checkFileExists("data/readings_normal.txt");
    checkFileExists("data/readings_flood.txt");
    checkFileExists("data/readings_critical.txt");
    checkFileExists("data/map.txt");
}

void SystemController::writeLog(const string &text) {
    ofstream log("data/log.txt", ios::app);
    if (!log) {
        throw SystemException("Cannot open data/log.txt");
    }
    log << projectName << " - session " << sessionCount << ": " << text << endl;
}

void SystemController::saveFinalReports() {
    double score;
    int level;
    int place;
    int i;

    if (get_summary(&score, &level, &place) == 0) {
        return;
    }

    RiskReport riskReport(place, score, (RiskLevel) level);
    riskReport.saveToFile("data/final_report.txt");

    for (i = 0; i < get_history_count(); i++) {
        Alert a = get_history_item(i);
        AlertReport alertReport(a.location_id, a.alert_id, a.level, a.confidence);
        alertReport.saveToFile("data/final_report.txt");
    }
    cout << "Final reports saved in data/final_report.txt" << endl;
}

void SystemController::start() {
    checkDataFiles();

    sessionCount++;
    writeLog("started");

    system_init();
    system_run();

    saveFinalReports();
    writeLog("ended");
}

int SystemController::getSessionCount() const {
    return sessionCount;
}
