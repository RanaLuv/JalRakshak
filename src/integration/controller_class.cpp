// controller_class.cpp - C++ part of Role 6 (Luv)

#include <iostream>
#include <fstream>
#include "controller_class.h"
#include "system_controller.h"   // the C menu functions

using namespace std;

// SystemException 

SystemException::SystemException(const string &msg) {
    message = msg;
}

const char *SystemException::what() const noexcept {
    return message.c_str();
}

//SystemController

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

// Tries to open the file. If it cannot, we throw an exception.
void SystemController::checkFileExists(const string &path) {
    ifstream file(path.c_str());
    if (!file) {
        throw SystemException("Missing file: " + path);
    }
}

void SystemController::checkDataFiles() {
    checkFileExists("data/readings_normal.txt");
    checkFileExists("data/readings_flood.txt");
}

// Adds one line at the end of data/log.txt
void SystemController::writeLog(const string &text) {
    ofstream log("data/log.txt", ios::app);
    if (!log) {
        throw SystemException("Cannot open data/log.txt");
    }
    log << projectName << " - session " << sessionCount << ": " << text << endl;
}

void SystemController::start() {
    checkDataFiles();            // stop early if the data files are missing

    sessionCount++;
    writeLog("started");

    system_init();               // C function
    system_run();                // C function: the menu

    writeLog("ended");
}

int SystemController::getSessionCount() const {
    return sessionCount;
}
