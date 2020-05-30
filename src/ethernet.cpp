#include <UIPEthernet.h>

static void (*reset)(void) = 0;

static void print_ip() {
    Serial.print(F("IP address: "));
    Serial.println(Ethernet.localIP());
}

void check_link() {
    if (Ethernet.linkStatus() != LinkON)
    {
        Serial.println(F("Connection lost."));
        reset();
    }
}

void handle_dhcp() {
    switch (Ethernet.maintain()) {
        case 1:
            //renewed fail
            Serial.println(F("Error: renewed fail"));
            reset();
            break;

        case 2:
            //renewed success
            print_ip();
            break;

        case 3:
            //rebind fail
            Serial.println(F("Error: rebind fail"));
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
  Serial.println(F("Setting up IP with DHCP..."));
  if (Ethernet.begin(mac) == 0) {
      Serial.println(F("DHCP failed, reset in 10 seconds..."));
      delay(10 * 1000);
      reset();
  }

  print_ip();
}
