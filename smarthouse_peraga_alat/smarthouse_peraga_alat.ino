// Include libraries
#include <BlynkSimpleEsp32.h> // include library
#include <WiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>
#include <MFRC522.h>

// Pin Definitions
#define OLED_RESET    -1
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64

#define RELAY1  12 // Lampu utama
#define RELAY2  13 // Lampu kejut
#define RELAY3  14 // Buzzer
#define RELAY4  27 // RFID

#define PIR_PIN  26
#define LED_GREEN 25
#define LED_RED   33

#define SS_PIN    21
#define RST_PIN   22

// OLED display object
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Blynk authentication token
#define BLYNK_TEMPLATE_ID           "1oBFmx6NvhENNCElTJEa09YNTxMXqflm"
#define BLYNK_TEMPLATE_NAME         "Mockingbird"
#define BLYNK_AUTH_TOKEN            "bakakaitou"       

char auth[] = "1oBFmx6NvhENNCElTJEa09YNTxMXqflm"; // masukan auth token yng didapatkan dari  
char ssid[] = "Mockingbird"; //masukan nama hotspot/Wifi yang digunakan
char pass[] = "bakakaitou"; //password WiFi

// RFID object
MFRC522 rfid(SS_PIN, RST_PIN);

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  
  // Initialize Blynk
  Blynk.begin(auth, ssid, pass);
  
  // Initialize OLED display
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    while(1);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  // Initialize pins
  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);
  pinMode(RELAY3, OUTPUT);
  pinMode(RELAY4, OUTPUT);
  pinMode(PIR_PIN, INPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  
  // Initialize RFID
  SPI.begin();
  rfid.PCD_Init();
  
  // Default screen
  displayDefault();
}

void loop() {
  Blynk.run();

  // Handle PIR sensor
  if(digitalRead(PIR_PIN) == HIGH) {
    triggerMotionDetected();
  }

  // Handle RFID
  if(rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    handleRFID();
  }
}

BLYNK_WRITE(V1) { // Blynk virtual pin to control Relay 1 (lampu utama)
  int pinValue = param.asInt();
  digitalWrite(RELAY1, pinValue);
  if(pinValue == HIGH) {
    displayMessage("lampu utama nyala");
  }
  displayDefault();
}

void triggerMotionDetected() {
  // Turn on lampu kejut and send notification
  digitalWrite(RELAY2, HIGH);
  Blynk.notify("Pergerakan terdeteksi!");
  displayMessage("ada apaan tuh");
  delay(3000); // Lampu menyala selama 3 detik
  digitalWrite(RELAY2, LOW);
  displayDefault();
}

void handleRFID() {
  String rfidTag = readRFIDTag();
  if(rfidTag == "your_correct_rfid_tag") {
    digitalWrite(LED_GREEN, HIGH);
    displayMessage("silahkan masuk mas");
    delay(3000);
    digitalWrite(LED_GREEN, LOW);
  } else {
    digitalWrite(RELAY3, HIGH); // Trigger buzzer
    digitalWrite(LED_RED, HIGH);
    displayMessage("siapa kamu?");
    Blynk.notify("Kartu RFID tidak dikenal!");
    delay(3000);
    digitalWrite(RELAY3, LOW);
    digitalWrite(LED_RED, LOW);
  }
  displayDefault();
}

String readRFIDTag() {
  String tag = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    tag.concat(String(rfid.uid.uidByte[i] < 0x10 ? " 0" : " "));
    tag.concat(String(rfid.uid.uidByte[i], HEX));
  }
  tag.toUpperCase();
  rfid.PICC_HaltA(); // Halt PICC
  rfid.PCD_StopCrypto1(); // Stop encryption on PCD
  return tag;
}

void displayMessage(String message) {
  display.clearDisplay();
  display.setCursor(0,0);
  display.println(message);
  display.display();
}

void displayDefault() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(":)");
  display.display();
}
