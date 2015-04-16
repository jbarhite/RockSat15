#include <Wire.h>
#define PWMpin 3
#define address 0x1E

String output;

void setup() {
  Serial.begin(9600);
  pinMode(PWMpin, OUTPUT);
  
  Wire.begin();
  Wire.beginTransmission(address); // open communication with HMC5883
  Wire.write(0x02); // select mode register
  Wire.write(0x00); // continuous measurement mode
  Wire.endTransmission();
}

void loop() {
  while (Serial.available() > 0) {
    int level = Serial.parseInt(); 
    if (Serial.read() == '\n') {
      level = constrain(level, 0, 255);
      analogWrite(PWMpin, level);
      Serial.print("PWM pin set to: ");
      Serial.println(level);
    }
  }
  
  delay(500);
  readMagnetometer();
  delay(500);
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
