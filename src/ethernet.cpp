#include <EthernetENC.h>

static void (*reset)(void) = 0;

static void print_ip() {
    Serial.print(F("IP: "));
    Serial.println(Ethernet.localIP());
}

void check_link() {
    if (Ethernet.linkStatus() != LinkON) {
        Serial.println(F("Conn lost"));
        reset();
    }
}

void handle_dhcp() {
    switch (Ethernet.maintain()) {
        case 1:
            //renewed fail
            Serial.println(F("Renew fail"));
            reset();
            break;

        case 2:
            //renewed success
            print_ip();
            break;

        case 3:
            //rebind fail
            Serial.println(F("Rebind fail"));
            reset();
            break;

        case 4:
            //rebind success
            print_ip();
            break;

        default:
            //nothing happened
            break;
    }
}

void setup_ethernet(const byte mac[]) {
    Serial.println(F("DHCP..."));
    if (Ethernet.begin(mac) == 0) {
        Serial.println(F("Fail"));
        delay(10 * 1000);
        reset();
    }

    print_ip();
}
