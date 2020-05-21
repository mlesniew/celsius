#include "sensor.h"
#include "serial_printf.h"

#define DS18B20_RESOLUTION 9

Sensor::Sensor(unsigned int pin) : pin(pin), one_wire(pin), sensors(&one_wire), connected(false) {}

bool Sensor::connect()
{
    // this searches for new devices
    sensors.begin();

    connected = false;
    const uint8_t device_count = sensors.getDeviceCount();
    if (!device_count) {
        printf("No devices found on pin %i.\n", pin);
        return false;
    }

    printf("Found %i devices on pin %i.\n", device_count, pin);
    for (uint8_t i = 0; i < device_count; ++i) {
        if (!sensors.getAddress(address, i)) {
            printf("Error reading address at index %i on pin %i.\n", i, pin);
            return false;
        }

        const bool supported = sensors.validFamily(address);

        printf("Device at index %i on pin %i has address %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x and %s a supported sensor.\n",
                i, pin,
                address[0], address[1], address[2], address[3], address[4], address[5], address[6], address[7],
                supported ? "is" : "is not");

        if (!supported)
            continue;

        if (sensors.getResolution(address) != DS18B20_RESOLUTION) {
            printf("Setting resolution to %i bits...\n", DS18B20_RESOLUTION);
            if (!sensors.setResolution(address, DS18B20_RESOLUTION)) {
                printf("Error setting resolution.\n");
                return false;
            }
        } else {
            printf("Resolution already set to %i bits.\n", DS18B20_RESOLUTION);
        }

        printf("Sensor attached to pin %i ready.\n", pin);
        connected = true;
        return true;
    }
    printf("No suitable sensor found on pin %i.\n", pin);
    return false;
}

void Sensor::begin() {
    printf("Initializing sensors on pin %i...\n", pin);
    sensors.begin();
    printf("Bus on pin %i is running on %s power.\n", pin, sensors.isParasitePowerMode() ? "parasite" : "normal");
    connect();
}

float Sensor::read() {
    float ret = DEVICE_DISCONNECTED_C;

    if (!connected && !connect()) {
        goto out;
    }

    if (!sensors.requestTemperaturesByAddress(address)) {
        connected = false;
        goto out;
    }

    ret = sensors.getTempC(address);

    if (ret == DEVICE_DISCONNECTED_C) {
        connected = false;
    }

out:
    return ret;
}
