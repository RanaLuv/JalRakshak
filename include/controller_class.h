#ifndef CONTROLLER_CLASS_H
#define CONTROLLER_CLASS_H

#include <string>
#include <exception>

class SystemException : public std::exception {
private:
    std::string message;
public:
    SystemException(const std::string &msg);
    const char *what() const noexcept;
};

class SystemController {
private:
    std::string projectName;
    int sessionCount;

    void checkFileExists(const std::string &path);
    void writeLog(const std::string &text);
    void saveFinalReports();

public:
    SystemController();
    SystemController(const std::string &name);
    ~SystemController();

    void checkDataFiles();
    void start();
    int getSessionCount() const;
};

#endif
