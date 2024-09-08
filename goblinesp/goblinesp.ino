#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "DHTesp.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <Arduino_JSON.h>
#include <ArduinoJson.h>

// Pins configuration
#define LDRanalog 34
#define pinLED1 33 
#define pinLED2 32 
#define pinLED3 25 
#define pinLED4 26 
#define DHT_PIN 18

// Water level sensors
const int level_air_penyiraman_akar = 12;
const int level_cairan_pestisida = 13;
const int level_cairan_pupuk = 14;

// LCD I2C configuration
LiquidCrystal_I2C lcd(0x27, 20, 4);

// WiFi credentials
const char ssid[] = "Mockingbird";
const char pass[] = "bakakaitou";

// MQTT server configuration
const char *mqttServer = "o697dff6.ala.asia-southeast1.emqxsl.com";
const char *mqtt_username = "SiJagoMerah_arjey";
const char *mqtt_password = "sijagomerah_arjey9314";
int port = 8883;

// MQTT topics
const char *topic_suhu = "monitoring/sensor/suhu";
const char *topic_kelembapan = "monitoring/sensor/kelembapan";
const char *topic_ldr = "monitoring/sensor/ldr";
const char *topic_level_air_penyiraman_akar = "monitoring/sensor/level_air_penyiraman_akar";
const char *topic_level_cairan_pestisida = "monitoring/sensor/level_cairan_pestisida";
const char *topic_level_cairan_pupuk = "monitoring/sensor/level_cairan_pupuk";
const char *topic_ph = "monitoring/sensor/ph";
const char *topic_tds = "monitoring/sensor/tds";
const char *topic_jarak = "monitoring/sensor/jarak";
const char *topic_waktu = "monitoring/sensor/waktu";
const char *topic_led_control = "monitoring/control/led";

float temperature, humidity;

unsigned long lastTime = 0;
unsigned long intervalTime = 2000;
int i = 0;

WiFiClientSecure espClient;
PubSubClient client(espClient);
DHTesp dht;

static const char *root_ca PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDrzCCApegAwIBAgIQCDvgVpBCRrGhdWrJWZHHSjANBgkqhkiG9w0BAQUFADBh
MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3
d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD
QTAeFw0wNjExMTAwMDAwMDBaFw0zMTExMTAwMDAwMDBaMGExCzAJBgNVBAYTAlVT
MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j
b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IENBMIIBIjANBgkqhkiG
9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4jvhEXLeqKTTo1eqUKKPC3eQyaKl7hLOllsB
CSDMAZOnTjC3U/dDxGkAV53ijSLdhwZAAIEJzs4bg7/fzTtxRuLWZscFs3YnFo97
nh6Vfe63SKMI2tavegw5BmV/Sl0fvBf4q77uKNd0f3p4mVmFaG5cIzJLv07A6Fpt
43C/dxC//AH2hdmoRBBYMql1GNXRor5H4idq9Joz+EkIYIvUX7Q6hL+hqkpMfT7P
T19sdl6gSzeRntwi5m3OFBqOasv+zbMUZBfHWymeMr/y7vrTC0LUq7dBMtoM1O/4
gdW7jVg/tRvoSSiicNoxBN33shbyTApOB6jtSj1etX+jkMOvJwIDAQABo2MwYTAO
BgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUA95QNVbR
TLtm8KPiGxvDl7I90VUwHwYDVR0jBBgwFoAUA95QNVbRTLtm8KPiGxvDl7I90VUw
DQYJKoZIhvcNAQEFBQADggEBAMucN6pIExIK+t1EnE9SsPTfrgT1eXkIoyQY/Esr
hMAtudXH/vTBH1jLuG2cenTnmCmrEbXjcKChzUyImZOMkXDiqw8cvpOp/2PV5Adg
06O/nVsJ8dWO41P0jmP6P6fbtGbfYmbW0W5BjfIttep3Sp+dWOIrWcBAI+0tKIJF
PnlUkiaY4IBIqDfv8NZ5YBberOgOzW6sRBc4L0na4UU+Krk2U886UAb3LujEV0ls
YSEY1QSteDwsOoBrp+uvFRTp2InBuThs4pFsiv9kuXclVzDAGySj4dzp30d8tbQk
CAUw7C29C79Fv1C5qfPrmAESrciIxpg0X40KPMbp1ZWVbd4=
-----END CERTIFICATE-----
)EOF";

void setup() {
  Serial.begin(115200);
  pinMode(pinLED1, OUTPUT);
  pinMode(pinLED2, OUTPUT);
  pinMode(pinLED3, OUTPUT);
  pinMode(pinLED4, OUTPUT);
  pinMode(LDRanalog, INPUT);
  pinMode(level_air_penyiraman_akar, INPUT);
  pinMode(level_cairan_pestisida, INPUT);
  pinMode(level_cairan_pupuk, INPUT);

  lcd.init();
  lcd.backlight();
  
  dht.setup(DHT_PIN, DHTesp::DHT22);
  
  wifi_init();
  mqtt_init();
}

void loop() {
  client.loop();
  if (!client.connected()) {
    mqtt_init();
  }
  send_data();
  if (Serial.available() > 0) {
    String dataReceived = Serial.readStringUntil('\n');
    lcd.clear(); // Hapus layar LCD
    lcd.setCursor(0, 0); // Atur kursor ke baris pertama, kolom pertama
    lcd.print(dataReceived); // Tampilkan data pada LCD
    handleSerialData(dataReceived);
    pusingpalagw(dataReceived);
  }
}

void wifi_init() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, pass, 6);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println(" Connected!");
}

void mqtt_init() {
  espClient.setCACert(root_ca);
  client.setServer(mqttServer, port);
  client.setCallback(mqtt_callback);
  while (!client.connected()) {
    String client_id = "sensor-";
    client_id += String(WiFi.macAddress());
    Serial.printf("The client %s connects to the public MQTT broker\n", client_id.c_str());
    if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("Public EMQX MQTT broker connected");
      client.subscribe(topic_suhu);
      client.subscribe(topic_kelembapan);
      client.subscribe(topic_ldr);
      client.subscribe(topic_level_air_penyiraman_akar);
      client.subscribe(topic_level_cairan_pestisida);
      client.subscribe(topic_level_cairan_pupuk);
      client.subscribe(topic_ph);
      client.subscribe(topic_tds);
      client.subscribe(topic_jarak);
      client.subscribe(topic_waktu);
      client.subscribe(topic_led_control);
    } else {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }
}

void send_data() {
  if ((millis() - lastTime) > intervalTime) {
    if (!client.connected()) {
      mqtt_init();
    }
    dht_suhu_read();
    dht_kelembaban_read();
    ldr_read();
    level_air_penyiraman_akar_read();
    level_cairan_pestisida_read();
    level_cairan_pupuk_read();
    lastTime = millis();
  }
}

void dht_suhu_read() {
  TempAndHumidity data = dht.getTempAndHumidity();
  if (!isnan(data.temperature)) {
    temperature = data.temperature;
    JSONVar dataObject;
    dataObject["temperature"] = temperature;
    String sData = JSON.stringify(dataObject);
    client.publish(topic_suhu, sData.c_str());
    lcd.setCursor(0, 1);
    lcd.print("Temp: ");
    lcd.print(temperature, 2);
    lcd.print(" C");
  } else {
    Serial.println("Failed to read from DHT sensor!");
  }
}

void dht_kelembaban_read() {
  TempAndHumidity data = dht.getTempAndHumidity();
  if (!isnan(data.humidity)) {
    humidity = data.humidity;
    JSONVar dataObject;
    dataObject["humidity"] = humidity;
    String sData = JSON.stringify(dataObject);
    client.publish(topic_kelembapan, sData.c_str());
    lcd.setCursor(0, 2);
    lcd.print("Humidity: ");
    lcd.print(humidity, 1);
    lcd.print(" %");
  } else {
    Serial.println("Failed to read from DHT sensor!");
  }
}

void ldr_read() {
  int nilaiADClux = analogRead(LDRanalog);
  float voltage = nilaiADClux * 5.0 / 4095.0;
  float resistance = 2000 * voltage / (1 - voltage / 5);
  float GAMMA = 0.9;
  float RL10 = 10000.0;
  float lux = pow(RL10 * 1e3 * pow(10, GAMMA) / resistance, (1 / GAMMA));
  JSONVar dataObject;
  dataObject["lux"] = lux;
  String sData = JSON.stringify(dataObject);
  client.publish(topic_ldr, sData.c_str());
  lcd.setCursor(0, 0);
  lcd.print("Cahaya: ");
  if (lux <= 200) {
    lcd.print("Gelap!!");
    digitalWrite(pinLED4, HIGH);
  } else {
    lcd.print("Terang!!");
    digitalWrite(pinLED4, LOW);
  }
}

void level_air_penyiraman_akar_read() {
  int sensorValue = analogRead(level_air_penyiraman_akar);
  controlLED(sensorValue, pinLED1);
  displayWaterLevel("Level air penyiraman akar", sensorValue);
  JSONVar dataObject;
  dataObject["level_air_penyiraman_akar"] = sensorValue;
  String sData = JSON.stringify(dataObject);
  client.publish(topic_level_air_penyiraman_akar, sData.c_str());
}

void level_cairan_pestisida_read() {
  int sensorValue = analogRead(level_cairan_pestisida);
  controlLED(sensorValue, pinLED2);
  displayWaterLevel("Level cairan pestisida", sensorValue);
  JSONVar dataObject;
  dataObject["level_cairan_pestisida"] = sensorValue;
  String sData = JSON.stringify(dataObject);
  client.publish(topic_level_cairan_pestisida, sData.c_str());
}

void level_cairan_pupuk_read() {
  int sensorValue = analogRead(level_cairan_pupuk);
  controlLED(sensorValue, pinLED3);
  displayWaterLevel("Level cairan pupuk", sensorValue);
  JSONVar dataObject;
  dataObject["level_cairan_pupuk"] = sensorValue;
  String sData = JSON.stringify(dataObject);
  client.publish(topic_level_cairan_pupuk, sData.c_str());
}

void controlLED(int sensorValue, int pinLED) {
  if (sensorValue < 734) { 
    digitalWrite(pinLED, LOW);
  } else if (sensorValue >= 734 && sensorValue < 1468) {
    digitalWrite(pinLED, HIGH);
  } else if (sensorValue >= 1468 && sensorValue < 2202) {
    digitalWrite(pinLED, HIGH);
  } else {
    digitalWrite(pinLED, HIGH);
  }
}

void displayWaterLevel(String description, int sensorValue) {
  Serial.print(description);
  Serial.print(": ");
  if (sensorValue < 734) {
    Serial.println("Empty");
  } else if (sensorValue >= 734 && sensorValue < 1468) {
    Serial.println("Low");
  } else if (sensorValue >= 1468 && sensorValue < 2202) {
    Serial.println("Medium");
  } else {
    Serial.println("High");
  }
}

void handleSerialData(String data) {
  data.trim();
  JSONVar dataObject;
  
  if (data.startsWith("PH:")) {
    String phValue = data.substring(3);
    dataObject["ph"] = phValue;
    String payload = JSON.stringify(dataObject);
    client.publish(topic_ph, payload.c_str());
  } else if (data.startsWith("TDS:")) {
    String tdsValue = data.substring(4);
    dataObject["tds"] = tdsValue;
    String payload = JSON.stringify(dataObject);
    client.publish(topic_tds, payload.c_str());
  } else if (data.startsWith("Distance:")) {
    String distanceValue = data.substring(9);
    dataObject["jarak"] = distanceValue;
    String payload = JSON.stringify(dataObject);
    client.publish(topic_jarak, payload.c_str());
  } else if (data.startsWith("Time:")) {
    String timeValue = data.substring(5);
    dataObject["waktu"] = timeValue;
    String payload = JSON.stringify(dataObject);
    client.publish(topic_waktu, payload.c_str());
  }
}

void pusingpalagw(String data) {
  // Menerjemahkan data yang diterima dari Arduino
  if (data.startsWith("Relay1:")) {
    String relay1Status = data.substring(7);
    if (relay1Status == "ON") {
      digitalWrite(pinLED1, HIGH); // Nyalakan relay 1
      Serial.println("RELAY1:ON"); // Kirim konfirmasi ke Arduino
    } else {
      digitalWrite(pinLED1, LOW); // Matikan relay 1
      Serial.println("RELAY1:OFF"); // Kirim konfirmasi ke Arduino
    }
  } else if (data.startsWith("Relay2:")) {
    String relay2Status = data.substring(7);
    if (relay2Status == "ON") {
      digitalWrite(pinLED2, HIGH); // Nyalakan relay 2
      Serial.println("RELAY2:ON"); // Kirim konfirmasi ke Arduino
    } else {
      digitalWrite(pinLED2, LOW); // Matikan relay 2
      Serial.println("RELAY2:OFF"); // Kirim konfirmasi ke Arduino
    }
  } else if (data.startsWith("Relay3:")) {
    String relay3Status = data.substring(7);
    if (relay3Status == "ON") {
      digitalWrite(pinLED3, HIGH); // Nyalakan relay 3
      Serial.println("RELAY3:ON"); // Kirim konfirmasi ke Arduino
    } else {
      digitalWrite(pinLED3, LOW); // Matikan relay 3
      Serial.println("RELAY3:OFF"); // Kirim konfirmasi ke Arduino
    }
  }
}

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);
  if (String(topic) == topic_led_control) {
    handleLEDControl(message);
  }
}

//void handleLEDControl(String message) {
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, message);
  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.f_str());
    return;
  }
  int led1 = doc["led1"];
  int led2 = doc["led2"];
  int led3 = doc["led3"];
  int led4 = doc["led4"];
  
  digitalWrite(pinLED1, led1);
  digitalWrite(pinLED2, led2);
  digitalWrite(pinLED3, led3);
  digitalWrite(pinLED4, led4);
  
  Serial.print("LED Control - LED1: ");
  Serial.print(led1);
  Serial.print(" LED2: ");
  Serial.print(led2);
  Serial.print(" LED3: ");
  Serial.print(led3);
  Serial.print(" LED4: ");
  Serial.println(led4);
}
