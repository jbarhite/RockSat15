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

  
}

void wait(int n) {
  delay(n);
  timestamp += n;
}
