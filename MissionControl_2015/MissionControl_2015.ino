#include <SD.h>
#include <EEPROM.h>
#include <Wire.h>

// SETTINGS **************************************************************************************************

#define PWMpin 3
#define ledPin 9
#define chipSelect 8
#define cameraPower 5
#define cameraButton 4
#define coilRelay 6

#define stateAddress 5
#define counterAddress 6
#define flashTime 150
#define waitTime 5 // time until Arduino activates cameras and Helmholtz coils -- must be less than 256 for coding reasons
#define cycles 3
#define RELAY_OPEN HIGH
#define RELAY_CLOSED LOW // relay board is active low
int allRelays[] = {cameraPower, cameraButton, coilRelay};

long data1[40]; // timestamp
int data2[40]; // mag x
int data3[40]; // mag y
int data4[40]; // mag z
int data5[40]; // battery voltage
int data6[40]; // thermistor

// OTHER VARIABLES *******************************************************************************************

boolean SDactive = false;
long timestamp = 0L;
int state;
int counter;
String output;

// FUNCTIONS *************************************************************************************************

void setup() {
  tempReset();
  
  // configure pins
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  pinMode(10, OUTPUT);
  for (int i=0; i<sizeof(allRelays) / sizeof(int); i++) {
    pinMode(allRelays[i], OUTPUT);
    digitalWrite(allRelays[i], RELAY_OPEN);
  }
  
  // initialize magnetometer
  Wire.begin();
  Wire.beginTransmission(0x1E); // open communication with HMC5883
  Wire.write(0x02); // select mode register
  Wire.write(0x00); // continuous measurement mode
  Wire.endTransmission();
  
  // initiate variables and write initial state to the mission log
  state = EEPROM.read(stateAddress);
  counter = EEPROM.read(counterAddress);
}

void loop() {
  // waiting to reach zero G
  if (state == 0) {
    wait(1000);
    counter++;
    EEPROM.write(counterAddress, counter);
    if (counter >= waitTime) {
      state = 1;
      EEPROM.write(stateAddress, 1);
      counter = 0;
      EEPROM.write(counterAddress, 0);
    }
  }
  
  // main experiment
  if (state > 0 && state <= cycles) {
    resetCamera();
    digitalWrite(coilRelay, RELAY_CLOSED);
    wait(1000); // **************************************************************** NEED TO LOG FLASH NUMBER AND TIMESTAMP ***********************
    rampUp(5);
    wait(1000);
    rampUp(10);
    wait(1000);
    rampUp(30);
    state++;
    EEPROM.write(stateAddress, state);
    turnOffCamera();
  }
  
  //if (state > cycles) { terminate(); }
}

void readSensors(int i) {
  int x, y, z;
  Wire.beginTransmission(0x1E);
  Wire.write(0x03);
  Wire.endTransmission();
  Wire.requestFrom(0x1E, 6);
  if (Wire.available() >= 6) {
    x = Wire.read()<<8;
    x |= Wire.read();
    z = Wire.read()<<8;
    z |= Wire.read();
    y = Wire.read()<<8;
    y |= Wire.read();
  }

  data1[i] = timestamp;
  data2[i] = x;
  data3[i] = y;
  data4[i] = z;
  data5[i] = analogRead(A0);
  data6[i] = analogRead(A1);
}

void turnOffCamera() {
  digitalWrite(cameraButton, RELAY_CLOSED);
  wait(1000);
  digitalWrite(cameraButton, RELAY_OPEN);
  wait(5000);
  digitalWrite(cameraPower, RELAY_OPEN);
}

void resetCamera() {
  digitalWrite(cameraPower, RELAY_OPEN);
  digitalWrite(cameraButton, RELAY_OPEN);
  wait(1000);
  digitalWrite(cameraPower, RELAY_CLOSED);
  wait(1000);
  digitalWrite(cameraButton, RELAY_CLOSED);
  wait(1000);
  digitalWrite(cameraButton, RELAY_OPEN);
  wait(8000);
}

void flash(int n) {
  for (int i=0; i<n; i++) {
    if (i > 0) { wait(flashTime); }
    digitalWrite(ledPin, HIGH);
    wait(flashTime);
    digitalWrite(ledPin, LOW);
  }
}

void rampUp(int duration) {
  flash(1);
  int frequency = 7;
  if (duration * frequency * 4 < 250) { frequency = 60 / duration; }
  for (int i=0; i<=255; i++) {
    analogWrite(PWMpin, i);
    if (i % frequency == 0) { readSensors(i / frequency); }
    wait(duration * 1000 / 255);
  }
  analogWrite(PWMpin, 0);
  writeDataToLog(255 / frequency);
}

void tempReset() {
  pinMode(2, OUTPUT);
  digitalWrite(2, LOW);
  pinMode(7, INPUT_PULLUP);
  if (digitalRead(7) == LOW) {
      EEPROM.write(stateAddress, 0);
      EEPROM.write(counterAddress, 0);
  }
}

void wait(int n) {
  delay(n);
  timestamp += n;
}

// SD CARD FUNCTIONS *****************************************************************************************

void writeDataToLog(int n) {
  SD.begin(chipSelect);
  File file = SD.open("data.txt", FILE_WRITE);
  if (file) {
    for (int i=0; i<=n; i++) {
      output = String(data1[i]);
      output += ",";
      output += String(data2[i]);
      output += ",";
      output += String(data3[i]);
      output += ",";
      output += String(data4[i]);
      output += ",";
      file.println(output);
      Serial.println(output);
    }
    file.close();
  }
}

String readContentsOfFile(char* fileName) {
  String data = "";
  if (!SDactive) { return data; }
  File file = SD.open(fileName);
  if (file) {
     while (file.available()) {
       data += char(file.read());
     }
  }
  return data;
}
