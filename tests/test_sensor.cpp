#include <iostream>
#include <vector>
#include "sensor.hpp"
using namespace std;

int main() {
    
    vector<Sensor*> sensors;
    sensors.push_back(new RainGauge());
    sensors.push_back(new WaterLevelSensor());
    for (int i = 0; i < 2; i++)
        cout << "reading: " << sensors[i]->read() << endl;

    
    DataManager dm(5);
    RainGauge rg;
    for (int i = 0; i < 8; i++) {
        SensorReading r = sim_generate(SENSOR_RAIN, 1, 0, i);
        r.value = rg.read();
        dm.add(r);
    }
    dm.show();
    
    try {
        SensorReading bad = sim_generate(SENSOR_RAIN, 1, 2, 0);
        dm.add(bad);
    } catch (InvalidReadingException &e) {
        cout << "Caught: " << e.what() << endl;
    }

    for (int i = 0; i < 2; i++) delete sensors[i];
    return 0;
}