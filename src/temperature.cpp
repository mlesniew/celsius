#include <Arduino.h>
#include <avr/wdt.h>
#include <UIPEthernet.h>

#include "sensor.h"

#define REBOOT_TIMEOUT (15 * 60 * 1000l)

#define SERVER_NAME "Ondogye"
#define BUFFER_SIZE 30
#define PIR_PIN A5
#define PIR_HOLD_TIME (60l * 1000)

char buffer[BUFFER_SIZE];

void check_link();
void handle_dhcp();
void setup_ethernet(const byte mac[]);

Sensor sensor[] = {2, 3, 4, 5, 6, 7, 8, 9};

bool pir_status = false;

#if __has_include("sensor_names.h")
#include "sensor_names.h"
#else
#warning "Using generic sensor names, please define custom names in sensor_names.h to override."
const char sensor_name_1[] PROGMEM = "1";
const char sensor_name_2[] PROGMEM = "2";
const char sensor_name_3[] PROGMEM = "3";
const char sensor_name_4[] PROGMEM = "4";
const char sensor_name_5[] PROGMEM = "5";
const char sensor_name_6[] PROGMEM = "6";
const char sensor_name_7[] PROGMEM = "7";
const char sensor_name_8[] PROGMEM = "8";
#endif

const char * const sensor_names[] PROGMEM = {
    sensor_name_1,
    sensor_name_2,
    sensor_name_3,
    sensor_name_4,
    sensor_name_5,
    sensor_name_6,
    sensor_name_7,
    sensor_name_8,
};

constexpr unsigned int sensor_count = sizeof(sensor) / sizeof(sensor[0]);

const byte mac[] = { 0x82, 0xc3, 0x34, 0x53, 0xe9, 0xd1 };

EthernetServer server(80);

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

  pinMode(PIR_PIN, INPUT_PULLUP);

  setup_ethernet(mac);
  server.begin();
  wdt_enable(WDTO_8S);
}

size_t read_until(EthernetClient & client, const char terminator) {
    size_t pos = 0;

    memset(buffer, 0, BUFFER_SIZE);

    while (client.connected()) {
        int data = client.read();

        if (data < 0) {
            // no more data available
            continue;
        }

        char c = (char) data;

        if (c == '\r') {
            // always ignore
            continue;
        }

        Serial.write(c);

        if (c == terminator) {
            // terminator found
            break;
        }

        if (pos < BUFFER_SIZE) {
            buffer[pos] = c;
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

bool handle_http() {
    EthernetClient client = server.available();
    if (!client)
        return false;

    uint16_t code = 200;

    // read the HTTP verb
    read_until(client, ' ');
    if (strncmp_P(buffer, (const char*) F("GET"), BUFFER_SIZE) != 0) {
        code = 400;
        goto consume;
    }

    // read uri
    read_until(client, ' ');
    if (strncmp_P(buffer, (const char*) F("/metrics"), BUFFER_SIZE) != 0) {
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
                "Content-Type: text/plain; charset=utf-8\r\n"
                "\r\n"));

    if (code < 300) {
        {
            send_data(
                    client,
                    F("# HELP presence PIR presence sensor activated\n"
                      "# TYPE presence gauge\n"
                      "presence "));
            send_data(client, pir_status);
            send_data(client, F("\n"));
        }

        // temperature sensors
        send_data(
                client,
                F("# HELP temperature Temperature in degrees Celsius\n"
                  "# TYPE temperature gauge\n")
                );

        for (unsigned int i = 0; i < sensor_count; ++i) {
            sensor[i].request_temperature();
        }

        delay(DS18B20_CONVERSION_DELAY_MS);

        for (unsigned int i = 0; i < sensor_count; ++i) {
            const double t = sensor[i].read();
            send_data(client, F("temperature{sensor=\""));
            {
                char name[16];
                strncpy_P(name, (const char*) pgm_read_dword(&(sensor_names[i])), 16);
                send_data(client, name);
            }
            send_data(client, F("\"} "));
            send_data(client, t);
            send_data(client, F("\n"));
        }
    }

    // wait for all data to get sent
    client.flush();

    // give the web browser time to receive the data
    delay(1);

    // close the connection:
    client.stop();

    return true;
}

void loop() {
    wdt_reset();

    {
        static unsigned long last_pir_active_time = 0;
        const auto pir = digitalRead(PIR_PIN) == LOW ? 1 : 0;
        if (pir) {
            pir_status = true;
            last_pir_active_time = millis();
        } else if (pir_status && (millis() - last_pir_active_time > PIR_HOLD_TIME)) {
            pir_status = false;
        }
    }

    check_link();
    handle_dhcp();
    bool http_client_handled = handle_http();

#ifdef REBOOT_TIMEOUT
    #warning The Arduino will reset automatically if no requests are received for REBOOT_TIMEOUT milliseconds
    {
        static constexpr unsigned long reboot_timeout = REBOOT_TIMEOUT;
        static unsigned long last_http_client = millis();

        if (http_client_handled) {
            last_http_client = millis();
        } else if (millis() - last_http_client > reboot_timeout) {
            // wait for the Watchdog to reset the board
            while (1);
        }
    }
#endif
}
