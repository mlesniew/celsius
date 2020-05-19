#include <printf.h>
#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define DS18B20_RESOLUTION 9

// this is needed for printf.h
void _putchar(char character) {
    Serial.write(character);
}

class Sensor {
public:
    Sensor(unsigned int pin) : pin(pin), one_wire(pin), sensors(&one_wire), connected(false) {
    }

    bool connect()
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

            printf("Device at index %i has address %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x and %s a supported sensor.\n",
                    pin,
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

    void begin() {
        printf("Initializing sensors on pin %i...\n", pin);
        sensors.begin();
        printf("Bus on pin %i is running on %s power.\n", pin, sensors.isParasitePowerMode() ? "parasite" : "normal");
        connect();
    }

    float read() {
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

protected:
    const unsigned int pin;
    OneWire one_wire;
    DallasTemperature sensors;
    bool connected;
    DeviceAddress address;
};

Sensor sensor[] = {2, 3};
const unsigned int sensor_count = sizeof(sensor) / sizeof(sensor[0]);

void setup(void) {
  Serial.begin(9600);
  printf("Initializing...\n");
  for (unsigned int i = 0; i < sensor_count; ++i)
      sensor[i].begin();
}

void loop(void)
{
  for (unsigned int i = 0; i < sensor_count; ++i) {
      const double t = sensor[i].read();
      printf("Temperature %i: %f\n", i, t);
  }

  delay(1000);
}
