#include <SD.h>
#include <EEPROM.h>
#include <Wire.h>

// SETTINGS **************************************************************************************************

#define PWM 3
#define ledPin 9
#define chipSelect 8
#define cameraPower 0
#define cameraButton 7

#define stateAddress 5
#define counterAddress 6
#define flashTime 150
#define waitTime 45 // time until Arduino activates cameras and Helmholtz coils -- must be less than 256 for coding reasons
#define RELAY_OPEN LOW
#define RELAY_CLOSED HIGH
int allRelays[] = {0, 1, 2};


// OTHER VARIABLES *******************************************************************************************

boolean SDactive = false;
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

  // attempt to initialize magnetometer
  SDactive = SD.begin(chipSelect)
  
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

void wait(int n) {
  delay(n);
  timestamp += n;
}
