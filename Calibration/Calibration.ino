#include <Wire.h>
#define PWMpin 3
#define address 0x1E

String output;
int level = -1;
int x_avg, z_avg, a0_avg, a2_avg;

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
    level = Serial.parseInt(); 
    if (Serial.read() == '\n') {
      level = constrain(level, -1, 255);
    }
  }
  
  if (level > -1) {
    analogWrite(PWMpin, level);
    Serial.print("PWM setting: ");
    Serial.println(level);
    x_avg = 0;
    z_avg = 0;
    a0_avg = 0;
    a2_avg = 0;
    delay(500);
    
    for (int i=0; i<9; i++) {
      readSensors();
      delay(250);
    }
    delay(250);
    
    Serial.println("Averages:");
    Serial.print(x_avg / 9.0);
    Serial.print(", ");
    Serial.print(z_avg / 9.0);
    Serial.print(", ");
    Serial.print(a0_avg / 9.0);
    Serial.print(", ");
    Serial.println(a2_avg / 9.0);
    Serial.println("");
    analogWrite(PWMpin, 0);
    level = -1;
  }
}

void readSensors() {
  int x, y, z, a0, a2;
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
  a0 = analogRead(A0);
  a2 = analogRead(A2);
  
  x_avg += x;
  z_avg += z;
  a0_avg += a0;
  a2_avg += a2;
  output = "";
  output += x;
  output += ",";
  output += z;
  output += ",";
  output += a0;
  output += ",";
  output += a2;
  Serial.println(output);
}
