#include <UIPEthernet.h>

#include "sensor.h"

Sensor sensor[] = {2, 3, 4, 5, 6, 7, 8, 9};
const unsigned int sensor_count = sizeof(sensor) / sizeof(sensor[0]);

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 1, 177);

// Initialize the Ethernet server library
// with the IP address and port you want to use 
// (port 80 is default for HTTP):
EthernetServer server(80);

void setup(void) {
  Serial.begin(9600);
  Serial.println("Initializing...\n");
  for (unsigned int i = 0; i < sensor_count; ++i)
      sensor[i].begin();

  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();
}

void loop() {
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    Serial.println(F("Got client"));
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.print(F(
          "HTTP/1.1 200 OK\r\n"
          "Content-Type: text/html\r\n"
          "\r\n"
          "<!DOCTYPE HTML>\r\n"
          "<html><meta http-equiv=\"refresh\" content=\"5\">"));
          for (unsigned int i = 0; i < sensor_count; ++i) {
              const double t = sensor[i].read();
              client.print(F("Temperature "));
              client.print(i);
              client.print(F(" is "));
              client.print(t);
              client.print(F("<br />"));
          }
          client.print(F("</html>"));
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } 
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
  }
}
