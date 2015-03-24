#include <SD.h>
#include <EEPROM.h>
#include <Wire.h>

// SETTINGS **************************************************************************************************
#define PWM 3


// OTHER VARIABLES *******************************************************************************************

long timestamp = 0L;

// FUNCTIONS *************************************************************************************************

void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}

void wait(int n) {
  delay(n);
  timestamp += n;
}
