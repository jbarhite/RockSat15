#include <SD.h>
#include <EEPROM.h>
#include <Wire.h>

//settings
int analogValue = 0;
#define address 0x1E
String output;

//functions
// initialize magnetometer
void setup() {
  Wire.begin();
  Wire.beginTransmission(address); // open communication with HMC5883
  Wire.write(0x02); // select mode register
  Wire.write(0x00); // continuous measurement mode
  Wire.endTransmission();
  
  Serial.begin(9600);

}

void loop() {
  readMagnetometer();
  // delay 10 milliseconds before the next reading:
  delay(10);
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
  Serial.println(output);
}
