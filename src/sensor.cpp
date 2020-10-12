#include "sensor.h"

Sensor::Sensor(uint8_t pin) : one_wire(pin), sensors(&one_wire), connected(false) {}

bool Sensor::connect() {
    sensors.begin();

    Serial.print(F("Scanning... "));
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
        Serial.println(F(" unsupported device"));
        return false;
    }

    if (sensors.getResolution(address) != DS18B20_RESOLUTION) {
        Serial.print(F(" configuring"));
        if (!sensors.setResolution(address, DS18B20_RESOLUTION)) {
            Serial.println(F(" error "));
            return false;
        }
    }

    // use asynchronous requests
    sensors.setWaitForConversion(false);

    Serial.println(F(" OK"));
    return true;
}

void Sensor::request_temperature() {
    connected = connected || connect();

    if (!connected) {
        // still no device
        return;
    }

    if (!sensors.requestTemperaturesByIndex(0)) {
        // disconnected again!
        connected = false;
    }
}

float Sensor::read() {
    float ret = DEVICE_DISCONNECTED_C;

    if (!connected) {
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
