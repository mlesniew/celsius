#ifndef SENSOR_H
#define SENSOR_H

#include <OneWire.h>
#include <DallasTemperature.h>

#define DS18B20_RESOLUTION 12
#define DS18B20_CONVERSION_DELAY_MS (750 / (1 << (12 - DS18B20_RESOLUTION)))

class Sensor {
public:
    Sensor(uint8_t pin);

    bool connect();
    void begin();
    void request_temperature();
    float read();
    bool reconnect_if_needed();

    const uint8_t pin;

protected:
    OneWire one_wire;
    DallasTemperature sensors;
    bool connected;
};

#endif
