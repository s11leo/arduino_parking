#include <SPI.h>
#include <Ethernet.h>

byte mac[] = { 0xE2, 0x65, 0xA3, 0xEF, 0xF5, 0x71 }; // Мак-адрес Ethernet
IPAddress ip(192, 168, 10, 10); // IP-адрес Arduino
byte gateway[] = { 192, 168, 10, 1 }; // IP-адрес шлюза
byte subnet[] = { 255, 255, 255, 0 }; // Маска подсети

EthernetServer server(80);

const int ID = 3;
const int sensorPin1 = 2; // Пин для первого датчика въезд
const int sensorPin2 = 3; // Пин для второго датчика въезд
const int sensorPin3 = 4; // Пин для первого датчика выезд
const int sensorPin4 = 5; // Пин для второго датчика выезд
const int buttonPin = 6;  // Пин для кнопки
const int relayPin = 7;   // Пин для реле

int triggerCount1 = 7; // Счетчик сработок датчиков въезд
int triggerCount2 = 2; // Счетчик сработок датчиков выезд

bool sensor1Active = false;
bool sensor2Active = false;
bool sensor3Active = false;
bool sensor4Active = false;
bool counting = false;

void checkEthernetConnection();
void handleButtonPress();

unsigned long lastEthernetCheckTime = 0;
const unsigned long ethernetCheckInterval = 60000; // Период проверки сети в миллисекундах (60 секунд)

void setup() {
  digitalWrite(2, HIGH); 
  digitalWrite(3, HIGH);
  digitalWrite(4, HIGH);
  digitalWrite(5, HIGH);
  pinMode(buttonPin, INPUT_PULLUP);  // Устанавливаем пин кнопки в режим входа с внутренним подтягивающим резистором
  pinMode(relayPin, OUTPUT);  // Устанавливаем пин реле в режим вывода

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

  // Проверка сработки датчиков въезда
  if (state1 == LOW && !sensor1Active) {
    sensor1Active = true;
    lastActivationTime = millis(); // Запоминаем время активации
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

    // Проверка сработки датчиков выезда
  if (state3 == LOW && !sensor3Active) {
    sensor3Active = true;
    lastActivationTime = millis(); // Запоминаем время активации
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
          client.println("Cars entered: " + String(triggerCount1)); // Сработки датчика въезда
          client.println("Cars left: " + String(triggerCount2)); // Сработки датчика выезда
          client.println("Total cars in the territory: " + String(totalTriggerCount)); // Сумма сработок всех датчиков
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

// Проверка состояния сетевого соединения и перезапуск Ethernet-модуля в случае пропажи сетевого соединения
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

// Обработка нажатия кнопки и управление реле
void handleButtonPress() {
  static bool buttonState = HIGH; // Текущее состояние кнопки
  static bool lastButtonState = HIGH; // Предыдущее состояние кнопки
  static unsigned long lastDebounceTime = 0;
  const int debounceDelay = 500; // Задержка антидребезга в миллисекундах

  int currentButtonState = digitalRead(buttonPin);

  if (currentButtonState != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (currentButtonState != buttonState) {
      buttonState = currentButtonState;
      
      if (buttonState == LOW) {
        // Кнопка нажата
        digitalWrite(relayPin, !digitalRead(relayPin)); // Инвертируем состояние реле
      }
    }
  }

  lastButtonState = currentButtonState;
}