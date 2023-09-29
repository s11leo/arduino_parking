#include <SPI.h>
#include <Ethernet.h>

byte mac[] = { 0xE2, 0x65, 0xA3, 0xEF, 0xF5, 0x01 }; // Ethernet MAC address
IPAddress ip(192, 168, 10, 7); // Arduino IP address
byte gateway[] = { 192, 168, 10, 1 }; // Gateway IP address
byte subnet[] = { 255, 255, 255, 0 }; // Subnet mask

EthernetServer server(80);

const int numSensors = 16;
const int ID = 1;
const int sensorPins[numSensors] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 14, 15, 16, 17, 18, 19}; // from 0 to 9 (0, 1, 2, 3, 4, 5, 6, 7, 8, 9) Digital A0 to A5-(14, 15, 16, 17, 18, 19)
int activatedSensors = 0;

// Variables and constants for debounce
const int debounceDelay = 500; // Debounce time in milliseconds
unsigned long lastDebounceTime[numSensors] = {0};
bool sensorState[numSensors] = {HIGH};

void setup() {
  pinMode(0, OUTPUT); 
  pinMode(1, OUTPUT); 
  pinMode(2, OUTPUT); 
  pinMode(3, OUTPUT); 
  pinMode(4, OUTPUT); // Cut the trace on the ethernet module before using the pin
  pinMode(5, OUTPUT); 
  pinMode(6, OUTPUT); 
  pinMode(7, OUTPUT); 
  pinMode(8, OUTPUT); 
  pinMode(9, OUTPUT); 
  pinMode(14, OUTPUT); 
  pinMode(15, OUTPUT); 
  pinMode(16, OUTPUT); 
  pinMode(17, OUTPUT); 
  pinMode(18, OUTPUT); 
  pinMode(19, OUTPUT); 
  
  // Start the Ethernet connection and the server
  Ethernet.begin(mac, ip, gateway, subnet);
  // Start the server
  server.begin();
}

void loop() {
  activatedSensors = 0;
  
  for (int i = 0; i < numSensors; i++) {
    int sensorReading = digitalRead(sensorPins[i]);
    
    if (sensorReading != sensorState[i]) {
      lastDebounceTime[i] = millis();
    }

    if ((millis() - lastDebounceTime[i]) > debounceDelay) {
      sensorState[i] = sensorReading;
      
      if (sensorState[i] == LOW) {
        activatedSensors++;
      }
    }
  }
  
  EthernetClient client = server.available();
  if (client) {
    bool currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        if (c == '\n' && currentLineIsBlank) {
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");
          client.println("Refresh: 1");
          client.println("Access-Control-Allow-Origin: *");
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          client.println("<head>");
          client.println("<title>");
          client.println("ID: " + String(ID));
          client.println("</title>");
          client.println("</head>");
          client.println("<body>");
          client.println("ID: " + String(ID));
          client.println("Busy Parks: " + String(activatedSensors) + " of " + String(numSensors)); // Output activated sensors
          client.println("</body>");
          client.println("</html>");
          break;
        }
        if (c == '\n') {
          currentLineIsBlank = true;
        } else if (c != '\r') {
          currentLineIsBlank = false;
        }
      }
    }
    delay(1);
    client.stop();
  }

  delay(1000); // Pause between updates in milliseconds
}
