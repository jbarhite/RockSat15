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

  // attempt to initialize magnetometer
  SDactive = SD.begin(chipSelect);
  
  // initialize magnetometer
  Wire.begin();
  Wire.beginTransmission(0x1E); // open communication with HMC5883
  Wire.write(0x02); // select mode register
  Wire.write(0x00); // continuous measurement mode
  Wire.endTransmission();
  
  // initiate variables and write initial state to the mission log
  state = EEPROM.read(stateAddress);
  counter = EEPROM.read(counterAddress);
  output = "Arduino ";
  if (counter > 0 || state > 0) { output += "re"; }
  output += "started. Initial state: ";
  output += state;
  output += ".";
  if (state == 0) {
    output += " Initial counter value: ";
    output += counter;
    output += ".";
  }
  writeToLog(output);
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
      output = "SPACE!!!";
      writeToLog(output);
    }
  }
  
  // main experiment
  if (state > 0 && state <= cycles) {
    resetCamera();
    digitalWrite(coilRelay, RELAY_CLOSED);
    wait(1000);
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

void readMagnetometer() {
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
  
  output = "Reading magnetometer.";
  writeToLog(output);
  output = String(timestamp);
  output += ",";
  output += x;
  output += ",";
  output += y;
  output += ",";
  output += z;
  output += ",";
  writeLineToSD("magdata.txt", output);
}

void turnOffCamera() {
  digitalWrite(cameraButton, RELAY_CLOSED);
  wait(1000);
  digitalWrite(cameraButton, RELAY_OPEN);
  wait(5000);
  digitalWrite(cameraPower, RELAY_OPEN);
  output = "Cameras turned off.";
  writeToLog(output);
}

void resetCamera() {
  output = "Resetting cameras.";
  writeToLog(output);
  digitalWrite(cameraPower, RELAY_OPEN);
  digitalWrite(cameraButton, RELAY_OPEN);
  wait(1000);
  digitalWrite(cameraPower, RELAY_CLOSED);
  wait(1000);
  digitalWrite(cameraButton, RELAY_CLOSED);
  wait(1000);
  digitalWrite(cameraButton, RELAY_OPEN);
  wait(8000);
  output = "Cameras recording.";
  writeToLog(output);
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
  output = "Beginning ramp (";
  output += duration;
  output += " seconds).";
  writeToLog(output);
  flash(1);
  int frequency = 6;
  if (duration * frequency * 4 < 250) { frequency = 60 / duration; }
  for (int i=0; i<=255; i++) {
    analogWrite(PWMpin, i);
    if (i % frequency == 0) { readMagnetometer(); }
    wait(duration * 1000 / 255);
  }
  analogWrite(PWMpin, 0);
  output = "Ramp completed.";
  writeToLog(output);
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

void writeLineToSD(char* fileName, String dataString) {
  if (!SDactive) {
    SDactive = SD.begin(chipSelect);
    if (!SDactive) { return; }
  }
  File file = SD.open(fileName, FILE_WRITE);
  if (file) {
    file.println(dataString);
    file.close();
  }
}

void writeToLog(String dataString) {
  dataString += " (";
  dataString += timestamp;
  dataString += ")";
  writeLineToSD("mission.txt", dataString);
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
