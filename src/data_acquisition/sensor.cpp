#ifndef SENSOR_cpp
#define SENSOR_cpp

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

#endif