#include "sensor.h"

#define DS18B20_RESOLUTION 9

Sensor::Sensor(uint8_t pin) : pin(pin), one_wire(pin), sensors(&one_wire), connected(false) {}

bool Sensor::connect()
{
    // this searches for new devices
    sensors.begin();

    Serial.print(F("P"));
    Serial.print(pin);
    Serial.print(sensors.isParasitePowerMode() ? F("/P:") : F("/N:"));

    DeviceAddress address;

    if (!sensors.getDeviceCount() || !sensors.getAddress(address, 0)) {
        Serial.println(F("no device"));
        return false;
    }

    for (uint8_t j = 0; j < 8; ++j) {
        Serial.print(address[j] >> 4, HEX);
        Serial.print(address[j] & 0xf, HEX);
    }

    if (!sensors.validFamily(address)) {
        Serial.println(F(" unsupported"));
        return false;
    }

    if (sensors.getResolution(address) != DS18B20_RESOLUTION) {
        Serial.print(F(" config"));
        if (!sensors.setResolution(address, DS18B20_RESOLUTION)) {
            Serial.println(F(" error"));
            return false;
        }
    }

    Serial.println(F(" OK"));
    connected = true;
    return connected;
}

void Sensor::begin() {
    sensors.begin();
    connect();
}

float Sensor::read() {
    float ret = DEVICE_DISCONNECTED_C;

    if (!connected && !connect()) {
        goto out;
    }

    if (!sensors.requestTemperaturesByIndex(0)) {
        connected = false;
        goto out;
    }

    ret = sensors.getTempCByIndex(0);

    if (ret == DEVICE_DISCONNECTED_C) {
        connected = false;
    }

out:
    if (!connected) {
        // this hack sets ret to NaN
        *((unsigned long *)(&ret)) = 0xffffffff;
    }
    return ret;
}
