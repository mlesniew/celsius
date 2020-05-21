#include "sensor.h"
#include "serial_printf.h"

Sensor sensor[] = {2, 3, 4, 5, 6, 7, 8, 9};
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
