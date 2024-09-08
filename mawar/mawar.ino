#include <Servo.h>
#define numOfValsRec 5
#define digitsPerValRec 1

Servo servoThumb;
Servo servoIndex;
Servo servoMiddle;
Servo servoRing;
Servo servoPinky;

int valsRec[numOfValsRec] = {0};  // Inisialisasi ke nilai default 0
int stringLength = numOfValsRec * digitsPerValRec + 1; //$00000
int counter = 0;
bool counterStart = false;
String receivedString = "";  // Inisialisasi sebagai string kosong

void setup() {
  Serial.begin(9600);
  servoThumb.attach(7);
  servoIndex.attach(9);
  servoMiddle.attach(11);
  servoRing.attach(8);
  servoPinky.attach(10);

  Serial.println("Setup complete. Waiting for data...");
}

void receiveData() {
  while (Serial.available()) {
    char c = Serial.read();

    if (c == '$') {
      counterStart = true;
      receivedString = "";  // Reset string ketika '$' diterima
      counter = 0;          // Reset counter ketika '$' diterima
    }
    if (counterStart) {
      if (counter < stringLength) {
        receivedString += c;  // Tambahkan karakter ke receivedString
        counter++;
      }
      if (counter >= stringLength) {
        //$00000
        for (int i = 0; i < numOfValsRec; i++) {
          int num = (i * digitsPerValRec) + 1;
          valsRec[i] = receivedString.substring(num, num + digitsPerValRec).toInt();
        }
        Serial.print("Received: ");
        Serial.println(receivedString);
        Serial.print("Parsed values: ");
        for (int i = 0; i < numOfValsRec; i++) {
          Serial.print(valsRec[i]);
          Serial.print(" ");
        }
        Serial.println();
        receivedString = "";
        counter = 0;
        counterStart = false;
      }
    }
  }
}

void loop() {
  receiveData();
  if (valsRec[0] == 1) { servoThumb.write(180); } else { servoThumb.write(0); }
  if (valsRec[1] == 1)  { servoIndex.write(180); } else { servoIndex.write(0); }
  if (valsRec[2] == 1) { servoMiddle.write(180); } else { servoMiddle.write(0); }
  if (valsRec[3] == 1) { servoRing.write(180); } else { servoRing.write(0); }
  if (valsRec[4] == 1) { servoPinky.write(180); } else { servoPinky.write(0); }
}
