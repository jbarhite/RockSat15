#include <SD.h>
#include <EEPROM.h>
#include <Wire.h>

// SETTINGS **************************************************************************************************
#define PWM 3


// OTHER VARIABLES *******************************************************************************************

boolean SDactive = false;
long timestamp = 0L;

// FUNCTIONS *************************************************************************************************

void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

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
