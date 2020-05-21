#ifndef SENSOR_H
#define SENSOR_H

#include <OneWire.h>
#include <DallasTemperature.h>

class Sensor {
public:
    Sensor(unsigned int pin);

    bool connect();
    void begin();
    float read();

    const unsigned int pin;

protected:
    OneWire one_wire;
    DallasTemperature sensors;
    bool connected;
    DeviceAddress address;
};


#endif
