#include <SPI.h>
#include <Ethernet.h>

byte mac[] = { 0xE2, 0x65, 0xA3, 0xEF, 0xF5, 0x71 }; // Ethernet MAC address
IPAddress ip(192, 168, 10, 10); // Arduino IP address
byte gateway[] = { 192, 168, 10, 1 }; // Gateway IP address
byte subnet[] = { 255, 255, 255, 0 }; // Subnet mask

EthernetServer server(80);

const int ID = 3;
const int sensorPin1 = 2; // Pin for the first entrance sensor
const int sensorPin2 = 3; // Pin for the second entrance sensor
const int sensorPin3 = 4; // Pin for the first exit sensor
const int sensorPin4 = 5; // Pin for the second exit sensor
const int buttonPin = 6;  // Pin for the button
const int relayPin = 7;   // Pin for the relay

int triggerCount1 = 7; // Entrance sensor trigger count
int triggerCount2 = 2; // Exit sensor trigger count

bool sensor1Active = false;
bool sensor2Active = false;
bool sensor3Active = false;
bool sensor4Active = false;
bool counting = false;

void checkEthernetConnection();
void handleButtonPress();

unsigned long lastEthernetCheckTime = 0;
const unsigned long ethernetCheckInterval = 60000; // Network check interval in milliseconds (60 seconds)

void setup() {
  digitalWrite(2, HIGH); 
  digitalWrite(3, HIGH);
  digitalWrite(4, HIGH);
  digitalWrite(5, HIGH);
  pinMode(buttonPin, INPUT_PULLUP);  // Set button pin to input mode with internal pull-up resistor
  pinMode(relayPin, OUTPUT);  // Set relay pin to output mode

  Ethernet.begin(mac, ip, gateway, subnet);

  server.begin();
  Serial.begin(9600);
}

void loop() {

  int state1 = digitalRead(sensorPin1);
  int state2 = digitalRead(sensorPin2);
  int state3 = digitalRead(sensorPin3);
  int state4 = digitalRead(sensorPin4);

  static unsigned long lastActivationTime = 0;
  const unsigned long timeout = 5000;

  // Check entrance sensors
  if (state1 == LOW && !sensor1Active) {
    sensor1Active = true;
    lastActivationTime = millis(); // Record activation time
  }

  if (sensor1Active && state2 == LOW) {
    Serial.println("Car_IN");
    triggerCount1++;
    sensor1Active = false;
  }

  if (sensor1Active && state2 == !LOW && millis() - lastActivationTime >= timeout) {
    Serial.println("Timeout_Car_IN");
    sensor1Active = false;
    sensor2Active = false;
  }

  // Check exit sensors
  if (state3 == LOW && !sensor3Active) {
    sensor3Active = true;
    lastActivationTime = millis(); // Record activation time
  }

  if (sensor3Active && state4 == LOW) {
    Serial.println("Car_OUT");
    triggerCount2--;
    sensor3Active = false;
  }

  if (sensor3Active && state4 == !LOW && millis() - lastActivationTime >= timeout) {
    Serial.println("Timeout_Car_OUT");
    sensor3Active = false;
    sensor4Active = false;
  }
  int totalTriggerCount = triggerCount1 + triggerCount2;

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
          client.println("Cars entered: " + String(triggerCount1)); // Entrance sensor triggers
          client.println("Cars left: " + String(triggerCount2)); // Exit sensor triggers
          client.println("Total cars in the territory: " + String(totalTriggerCount)); // Sum of all sensor triggers
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

  delay(1000);
}

// Check network connection status and restart Ethernet module if connection is lost
void checkEthernetConnection() {
  unsigned long currentMillis = millis();

  if (currentMillis - lastEthernetCheckTime >= ethernetCheckInterval) {
    lastEthernetCheckTime = currentMillis;

    if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Ethernet connection lost. Restarting Ethernet...");
      Ethernet.begin(mac, ip, gateway, subnet);
    }
  }
}

// Handle button press and control the relay
void handleButtonPress() {
  static bool buttonState = HIGH; // Current button state
  static bool lastButtonState = HIGH; // Previous button state
  static unsigned long lastDebounceTime = 0;
  const int debounceDelay = 500; // Debounce delay in milliseconds

  int currentButtonState = digitalRead(buttonPin);

  if (currentButtonState != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (currentButtonState != buttonState) {
      buttonState = currentButtonState;
      
      if (buttonState == LOW) {
        // Button is pressed
        digitalWrite(relayPin, !digitalRead(relayPin)); // Toggle relay state
      }
    }
  }

  lastButtonState = currentButtonState;
}
