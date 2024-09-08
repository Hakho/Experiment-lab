#define BLYNK_TEMPLATE_ID "TMPL6Y3l7NKu0"
#define BLYNK_TEMPLATE_NAME "SmartHouse"
#define BLYNK_AUTH_TOKEN "_fOpfqjeJ14eF5tDUh7o4dIlfYAcVOTo"

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// Pin definitions
#define RELAY_1 5
#define RELAY_2 18
#define RELAY_3 19
#define RELAY_4 23
#define PIR_SENSOR 27

int pirValue;
unsigned long pirLastDetectedTime = 0;
const unsigned long PIR_ACTIVE_DURATION = 5000; // 5 seconds in milliseconds
unsigned long lastDebugTime = 0;
const unsigned long DEBUG_INTERVAL = 1000; // Print debug info every 1 second

char ssid[] = "Mockingbird";
char pass[] = "bakakaitou";

void setup() {
  Serial.begin(115200);
  
  pinMode(PIR_SENSOR, INPUT);
  pinMode(RELAY_1, OUTPUT);
  pinMode(RELAY_2, OUTPUT);
  pinMode(RELAY_3, OUTPUT);
  pinMode(RELAY_4, OUTPUT);
  
  digitalWrite(RELAY_1, HIGH);
  digitalWrite(RELAY_2, HIGH);
  digitalWrite(RELAY_3, HIGH);
  digitalWrite(RELAY_4, HIGH);
  
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Serial.println("Connected to Blynk");
}

void handlePIR() {
  pirValue = digitalRead(PIR_SENSOR);
  Blynk.virtualWrite(V1, pirValue);
  
  if (pirValue == HIGH) {
    pirLastDetectedTime = millis();
    digitalWrite(RELAY_2, LOW);  // Activate Relay 2
    Blynk.virtualWrite(V4, 1);  // Update Blynk app
    Serial.println("Motion detected! Activating Relay 2");
  } else if (millis() - pirLastDetectedTime >= PIR_ACTIVE_DURATION) {
    digitalWrite(RELAY_2, HIGH);  // Deactivate Relay 2 after 5 seconds
    Blynk.virtualWrite(V4, 0);  // Update Blynk app
    Serial.println("No motion for 5 seconds. Deactivating Relay 2");
  }
}

void printDebugInfo() {
  if (millis() - lastDebugTime >= DEBUG_INTERVAL) {
    lastDebugTime = millis();
    Serial.println("----Debug Info----");
    Serial.print("PIR Sensor (pin ");
    Serial.print(PIR_SENSOR);
    Serial.print("): ");
    Serial.println(pirValue == HIGH ? "HIGH (Motion detected)" : "LOW (No motion)");
    Serial.print("Relay 2 (pin ");
    Serial.print(RELAY_2);
    Serial.print("): ");
    Serial.println(digitalRead(RELAY_2) == LOW ? "ON" : "OFF");
    Serial.print("Time since last motion: ");
    Serial.print((millis() - pirLastDetectedTime) / 1000);
    Serial.println(" seconds");
    Serial.println("-----------------");
  }
}

void loop() {
  Blynk.run();
  handlePIR();
  printDebugInfo();
}

BLYNK_WRITE(V0) {
  int button1Value = param.asInt();
  digitalWrite(RELAY_1, button1Value);
  Serial.print("Relay 1 set to: ");
  Serial.println(button1Value == 0 ? "ON" : "OFF");
}

BLYNK_WRITE(V2) {
  int button3Value = param.asInt();
  digitalWrite(RELAY_3, button3Value);
  Serial.print("Relay 3 set to: ");
  Serial.println(button3Value == 0 ? "ON" : "OFF");
}

BLYNK_WRITE(V3) {
  int button4Value = param.asInt();
  digitalWrite(RELAY_4, button4Value);
  Serial.print("Relay 4 set to: ");
  Serial.println(button4Value == 0 ? "ON" : "OFF");
}