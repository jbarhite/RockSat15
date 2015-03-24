#include <SD.h>
#include <EEPROM.h>
#include <Wire.h>

// SETTINGS **************************************************************************************************
#define PWM 3
#define ledPin 9
#define chipSelect 8
#define cameraPower 0
#define cameraButton 7
#define coilRelay 4

#define stateAddress 5
#define counterAddress 6
#define flashTime 150
#define waitTime 45 // time until Arduino activates cameras and Helmholtz coils -- must be less than 256 for coding reasons
#define RELAY_OPEN LOW
#define RELAY_CLOSED HIGH
int allRelays[] = {cameraPower, cameraButton, coilRelay}

// OTHER VARIABLES *******************************************************************************************

long timestamp = 0L;

// FUNCTIONS *************************************************************************************************

void setup() {
  // configure pins
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  pinMode(10, OUTPUT);
  for (int i=0; i<sizeof(allRelays) / sizeof(int); i++) {
    pinMode(allRelays[i], OUTPUT);
    digitalWrite(allRelays[i], RELAY_OPEN);
  }

  // initiate SD card, reset card (and don't do anything else) if there is a file named "reset.txt" whose only content is "RESET"
  if (!SD.begin(chipSelect)) {
    noLoop = true;
    return;
  }
  
  // initialize magnetometer
  Wire.begin();
  Wire.beginTransmission(address); // open communication with HMC5883
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
  if (noLoop) { return; }
  
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
      output = "SPAAAAAAAAAAAAAAAAAAACE.";
      writeToLog(output);
    }
  }
  // main experiment
  if (state > 0 && state <= cycles) {
    resetCamera();
    for (int i=0; i<=255; i++) {
		analogWrite(PWM, i);
		delay(10);
	}
	delay(2000);
	for (int i=255; i>=255; i--) {
		analogWrite(PWM, i);
		delay(10);
	}
	delay(2000);

      readMagnetometer();
      
      if (coilRelays[field] >= 0) {
        digitalWrite(coilRelays[field], RELAY_OPEN);
        output = "Relay on pin ";
        output += coilRelays[field];
        output += " opened.";
      }
      else { output = "No current to coils (end)."; }
      writeToLog(output);
      wait(1000);
      readMagnetometer();
    }
    turnOffCamera();
    state++;
    EEPROM.write(stateAddress, state);
  }
  
  if (state > cycles) { terminate(); }
}

void readMagnetometer() {
  int x, y, z;
  Wire.beginTransmission(address);
  Wire.write(0x03);
  Wire.endTransmission();
  Wire.requestFrom(address, 6);
  if (Wire.available() >= 6) {
    x = Wire.read()<<8;
    x |= Wire.read();
    z = Wire.read()<<8;
    z |= Wire.read();
    y = Wire.read()<<8;
    y |= Wire.read();
  }
  output = "Magnetometer reading: x: ";
  output += x;
  output += ", y: ";
  output += y;
  output += ", z: ";
  output += z;
  writeToLog(output);
}

void turnOffCamera() {
  digitalWrite(cameraButton, RELAY_CLOSED);
  wait(1000);
  digitalWrite(cameraButton, RELAY_OPEN);
  wait(2000);
  digitalWrite(cameraPower, RELAY_OPEN);
  output = "Cameras turned off.";
  writeToLog(output);
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

void writeLineToSD(char* fileName, String dataString) {
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
  File file = SD.open(fileName);
  String data = "";
  if (file) {
     while (file.available()) {
       data += char(file.read());
     }
  }
  return data;
}

void terminate() {
  state = cycles + 1;
  noLoop = true;
  EEPROM.write(stateAddress, state);
  output = "Experiment terminated.";
  writeToLog(output);
}

// resets state and counter and clears mission log
void totalReset() {
  EEPROM.write(stateAddress, 0);
  EEPROM.write(counterAddress, 0);
  SD.remove("mission.txt");
  File file = SD.open("mission.txt", FILE_WRITE);
  if (file) { file.close(); }
  writeLineToSD("reset.txt", " SUCCESSFUL");
}

void wait(int n) {
  delay(n);
  timestamp += n;
}
