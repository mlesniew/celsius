#include <UIPEthernet.h>

#include "sensor.h"

#define HOSTNAME "Ondogye"

Sensor sensor[] = {2, 3, 4, 5, 6, 7, 8, 9};
const unsigned int sensor_count = sizeof(sensor) / sizeof(sensor[0]);

byte mac[] = { 0xde, 0xad, 0xbe, 0xef, 0xfe, 0xed };

EthernetServer server(80);

void (*reset)(void) = 0;

void setup(void) {
  Serial.begin(9600);
  Serial.println("Initializing...\n");
  for (unsigned int i = 0; i < sensor_count; ++i)
      sensor[i].begin();

  Serial.println(F("Setting up IP with DHCP..."));
  if (Ethernet.begin(mac) == 0) {
      Serial.println(F("DHCP failed, reset in 10 seconds..."));
      delay(10 * 1000);
      reset();
  }

  Serial.print(F("IP address: "));
  Serial.println(Ethernet.localIP());

  server.begin();
}

#define BUFFER_MAX 50

size_t read_until(EthernetClient & client, const char terminator, char * buf = nullptr) {
    size_t pos = 0;

    if (buf)
        buf[0] = 0;

    while (client.connected() && (pos < BUFFER_MAX - 1)) {
        if (!client.available()) {
            // wait for more data
            continue;
        }

        char c = client.read();

        if (c == '\r') {
            // always ignore
            continue;
        }

        Serial.write(c);

        if (c == terminator) {
            // terminator found
            break;
        }

        if (buf && (pos < (BUFFER_MAX - 1))) {
            buf[pos] = c;
            buf[pos + 1] = 0;
        }

        ++pos;
    }

    return pos;
}

void handle_http() {
    EthernetClient client = server.available();
    if (!client)
        return;

    uint16_t code = 200;
    char buffer[BUFFER_MAX];

    // read the HTTP verb
    read_until(client, ' ', buffer);
    if (strcmp_P(buffer, (const char*) F("GET")) != 0) {
        code = 400;
        goto consume;
    }

    // read uri
    read_until(client, ' ', buffer);
    if (strcmp_P(buffer, (const char*) F("/metrics")) != 0) {
        code = 404;
        // goto consume;
    }

consume:
    // we got all we need now, let's read the remaining lines of headers ignoring their content
    while (read_until(client, '\n')) {
        // got another non-empty line, continue
    }

reply:
    // ready to reply
    client.print(F("HTTP/1.1 "));
    client.print(code);
    client.print(code < 300 ? F(" OK") : F(" Error"));
    client.print(F("\r\n"
                "Server: " HOSTNAME "\r\n"
                "Content-Type: text/plain\r\n"
                "\r\n"));

    if (code < 300) {
        client.print(
                F("# HELP temperature Temperature in degrees Celsius\n"
                  "# TYPE temperature gauge\n"));
        for (unsigned int i = 0; i < sensor_count; ++i) {
            const double t = sensor[i].read();
            client.print(F("temperature{sensor="));
            client.print(i);
            client.print(F("} "));
            client.print(t);
            client.print(F("\n"));
        }
    }

    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
}

void loop() {
    handle_http();

    switch (Ethernet.maintain()) {
        case 1:
            //renewed fail
            Serial.println(F("Error: renewed fail"));
            reset();
            break;

        case 2:
            //renewed success
            Serial.print(F("My IP address: "));
            Serial.println(Ethernet.localIP());
            break;

        case 3:
            //rebind fail
            Serial.println(F("Error: rebind fail"));
            reset();
            break;

        case 4:
            //rebind success
            Serial.print(F("My IP address: "));
            Serial.println(Ethernet.localIP());
            break;

        default:
            //nothing happened
            break;
    }
}
