// controller_class.h - C++ part of Role 6 (Luv)
// The class SystemController controls the whole program.
// It uses the C menu code (system_controller.c) inside it.

#ifndef CONTROLLER_CLASS_H
#define CONTROLLER_CLASS_H

#include <string>
#include <exception>

// Our own exception class. It inherits from the standard exception class.
class SystemException : public std::exception {
private:
    std::string message;
public:
    SystemException(const std::string &msg);
    const char *what() const noexcept;
};

class SystemController {
private:
    std::string projectName;   // name of the project
    int sessionCount;          // how many times the program was started

    void checkFileExists(const std::string &path);   // throws SystemException
    void writeLog(const std::string &text);          // writes to data/log.txt

public:
    SystemController();                              // default constructor
    SystemController(const std::string &name);       // constructor with a name
    ~SystemController();                             // destructor

    void checkDataFiles();       // checks that the input files are there
    void start();                // runs the whole program
    int getSessionCount() const;
};

#endif
