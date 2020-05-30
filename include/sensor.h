#ifndef SENSOR_H
#define SENSOR_H

#include <OneWire.h>
#include <DallasTemperature.h>

class Sensor {
public:
    Sensor(uint8_t pin);

    bool connect();
    void begin();
    float read();
    bool reconnect_if_needed();

    const uint8_t pin;

protected:
    OneWire one_wire;
    DallasTemperature sensors;
    bool connected;
};

#endif
