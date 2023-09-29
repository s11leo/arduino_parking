#include <SPI.h>
#include <Ethernet.h>

byte mac[] = { 0xE2, 0x65, 0xA3, 0xEF, 0xF5, 0x01 }; // Мак-адрес Ethernet
IPAddress ip(192, 168, 10, 7); // IP-адрес Arduino
byte gateway[] = { 192, 168, 10, 1 }; // IP-адрес шлюза
byte subnet[] = { 255, 255, 255, 0 }; // Маска подсети

EthernetServer server(80);

const int numSensors = 16;
const int ID = 1;
const int sensorPins[numSensors] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 14, 15, 16, 17, 18, 19}; // с 0 по 9 (0, 1, 2, 3, 4, 5, 6, 7, 8, 9) Цифровые А0 по А5-(14, 15, 16, 17, 18, 19)
int activatedSensors = 0;

// Переменные и константы для антидребезга
const int debounceDelay = 500; // Время антидребезга в миллисекундах
unsigned long lastDebounceTime[numSensors] = {0};
bool sensorState[numSensors] = {HIGH};

void setup() {
digitalWrite(0, HIGH);
digitalWrite(1, HIGH);  
digitalWrite(2, HIGH);
digitalWrite(3, HIGH);
digitalWrite(4, HIGH); // перед использованием пина отреж дорожку на ethernet модуле
digitalWrite(5, HIGH);
digitalWrite(6, HIGH);
digitalWrite(7, HIGH);
digitalWrite(8, HIGH);
digitalWrite(9, HIGH);
digitalWrite(14, HIGH);
digitalWrite(15, HIGH);
digitalWrite(16, HIGH);
digitalWrite(17, HIGH);
digitalWrite(18, HIGH);
digitalWrite(19, HIGH);

  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip, gateway, subnet);

  // start the server
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
          client.println("Busy Parks: " + String(activatedSensors) + " of " + String(numSensors)); // Вывод активированных датчиков
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

   delay(1000); // Пауза между обновлениями в миллисекундах
  
}
