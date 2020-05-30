#include <UIPEthernet.h>

#include "sensor.h"

#define SERVER_NAME "Ondogye"
#define SENSOR_RECONNECT_INTERVAL 60
#define BUFFER_MAX 50

void check_link();
void handle_dhcp();
void setup_ethernet(const byte mac[]);

Sensor sensor[] = {2, 3, 4, 5, 6, 7, 8, 9};
const unsigned int sensor_count = sizeof(sensor) / sizeof(sensor[0]);

const byte mac[] = { 0x82, 0xc3, 0x34, 0x53, 0xe9, 0xd1 };

EthernetServer server(80);

void reconnect_sensors() {
    static auto last_reconnect = millis();

    if (millis() - last_reconnect < 1000 * SENSOR_RECONNECT_INTERVAL) {
        return;
    }

    Serial.println("Reconnecting sensors...");

    for (unsigned int i = 0; i < sensor_count; ++i) {
        sensor[i].reconnect_if_needed();
    }

    last_reconnect = millis();
}

void setup() {
  Serial.begin(9600);
  Serial.println(F(
       " _____       _\n"
       "|     |___ _| |___ ___ _ _ ___ \n"
       "|  |  |   | . | . | . | | | -_|\n"
       "|_____|_|_|___|___|_  |_  |___|\n"
       "                  |___|___|\n"
       "\n"
       SERVER_NAME " " __DATE__ " " __TIME__ "\n"));

  for (unsigned int i = 0; i < sensor_count; ++i)
      sensor[i].begin();

  setup_ethernet(mac);
  server.begin();
}

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

template <typename T>
void send_data(EthernetClient & client, const T & data) {
    Serial.print(data);
    client.print(data);
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

    // ready to reply
    send_data(client, F("HTTP/1.1 "));
    send_data(client, code);
    send_data(client, code < 300 ? F(" OK") : F(" Error"));
    send_data(client, F("\r\n"
                "Server: " SERVER_NAME "\r\n"
                "Content-Type: text/plain\r\n"
                "\r\n"));

    if (code < 300) {
        send_data(
                client,
                F("# HELP temperature Temperature in degrees Celsius\n"
                  "# TYPE temperature gauge\n"));
        for (unsigned int i = 0; i < sensor_count; ++i) {
            const double t = sensor[i].read();
            send_data(client, F("temperature{sensor=\""));
            send_data(client, i);
            send_data(client, F("\"} "));
            send_data(client, t);
            send_data(client, F("\n"));
        }
    }

    // give the web browser time to receive the data
    delay(1);

    // close the connection:
    client.stop();
}

void loop() {
    reconnect_sensors();
    check_link();
    handle_dhcp();
    handle_http();
}
