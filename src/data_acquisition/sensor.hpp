#ifndef SENSOR_HPP
#define SENSOR_HPP

#include <iostream>
extern "C" {
#include "data_acquisition.h"
}

class Sensor {
public:
    virtual ~Sensor() {}
    virtual double read() = 0;
};

class RainGauge : public Sensor {
    int calls = 0;
public:
    double read() override { return 2.0 + (calls++ % 3); }
};

class WaterLevelSensor : public Sensor {
    int calls = 0;
public:
    double read() override { return 1.0 + 0.1 * (calls++ % 3); }
};

class InvalidReadingException {
public:
    const char *what() { return "Invalid reading rejected"; }
};

class DataManager {
    CircularBuffer *buf;
public:
    DataManager(int cap) { buf = cb_create(cap); }
    ~DataManager() { cb_free(buf); }

    void add(SensorReading r) {
        if (validate_reading(&r) != STATUS_OK)
            throw InvalidReadingException();
        cb_push(buf, r);
    }

    void show() {
        SensorReading out[100];
        int n = cb_get_last(buf, 100, out);
        for (int i = 0; i < n; i++)
            std::cout << out[i].value << " ";
        std::cout << std::endl;
    }
};

#endif