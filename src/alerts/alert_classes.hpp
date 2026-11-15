#ifndef ALERT_CLASSES_HPP
#define ALERT_CLASSES_HPP

#include <stdexcept>
#include <string>
#include "alerts.h"


namespace jalrakshak {

class EmptyQueueException : public std::runtime_error {
public:
    EmptyQueueException() : std::runtime_error("No pending alerts in the queue") {}
};

class Alert {
protected:
    ::Alert data_;
public:
    explicit Alert(const ::Alert& data) : data_(data) {}
    virtual ~Alert() = default;
    virtual std::string message() const;
    const ::Alert& data() const { return data_; }
};

class FloodAlert : public Alert {
public:
    using Alert::Alert;
    std::string message() const override;
};

class LandslideAlert : public Alert {
public:
    using Alert::Alert;
    std::string message() const override;
};

class AlertManager {
private:
    AlertHeap *heap_;
    AlertHistory *history_;
public:
    explicit AlertManager(int capacity = ALERT_DEFAULT_CAPACITY);
    ~AlertManager();
    AlertManager(const AlertManager&) = delete;
    AlertManager& operator=(const AlertManager&) = delete;
    Status enqueue(const ::Alert& alert);
    ::Alert next();
    Status saveHistory(const std::string& path) const;
    Status loadHistory(const std::string& path);
    int pending() const;
    int historyCount() const;
};

} 

#endif
