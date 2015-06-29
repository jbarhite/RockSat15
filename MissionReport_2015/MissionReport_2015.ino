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
#define waitTime 45 // time until Arduino activates cameras and Helmholtz coils -- must be less than 256 for coding reasons
#define cycles 3
#define RELAY_OPEN HIGH
#define RELAY_CLOSED LOW // relay board is active low
int allRelays[] = {cameraPower, cameraButton, coilRelay};

// OTHER VARIABLES *******************************************************************************************

boolean backupData = false;
long timestamp = 0L;
int state;
int counter;
String output;

// FUNCTIONS *************************************************************************************************

void setup() {
  Serial.begin(9600);
  mlog("Check pins 2/7 for reset signal.");
  mlog("Configure pins.");
  mlog("Initialize magnetometer.");
  mlog("Initialize state variables from EEPROM.");
}

void loop() {
  // waiting to reach zero G
  if (state == 0) {
    wait(1000);
    counter++;
    EEPROM.write(counterAddress, counter);
    mlog("Increment counter to " + String(counter));
    if (counter >= waitTime) {
      state = 1;
      EEPROM.write(stateAddress, 1);
      counter = 0;
      EEPROM.write(counterAddress, 0);
      mlog("Set state to 1 and counter to 0.");
    }
  }
  
  // main experiment
  if (state > 0 && state <= cycles) {
    mlog("Call camera reset.");
    resetCamera();
    mlog("Flash " + String(state) + " time(s).");
    flash(state);
    mlog("Close coil relay.");
    wait(1000);
    if (state == 1) {
      mlog("Set PWM pin to 255.");
      wait(5000);
      mlog("Set PWM pin to 0.");
    }
    mlog("Call 5 second ramp.");
    rampUp(5);
    wait(1000);
    mlog("Call 10 second ramp.");
    rampUp(10);
    wait(1000);
    mlog("Call 30 second ramp.");
    rampUp(30);
    state++;
    EEPROM.write(stateAddress, state);
    mlog("Increment state to " + String(state));
    if (!backupData) { mlog("Write backup data to EEPROM."); }
    mlog("Open coil relay.");
    mlog("Call camera shut-off.");
    turnOffCamera();
  }
}

void readSensors(int i) {
/*  int x, y, z;
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
  data6[i] = analogRead(A2);*/
}

void turnOffCamera() {
  mlog("Close camera button relay.");
  wait(1000);
  mlog("Open camera button relay.");
  wait(5000);
  mlog("Open camera power relay.");
}

void resetCamera() {
  mlog("Open camera power relay.");
  mlog("Open camera button relay.");
  wait(1000);
  mlog("CLose camera power relay.");
  wait(1000);
  mlog("Close camera button relay.");
  wait(1000);
  mlog("Open camera button relay.");
  wait(8000);
}

void flash(int n) {
  for (int i=0; i<n; i++) {
    if (i > 0) { wait(flashTime); }
    wait(flashTime);
  }
}

void rampUp(int duration) {
  int frequency = 7;
  if (duration * frequency * 4 < 250) { frequency = 60 / duration; }
  for (int i=0; i<=255; i++) {
    mlog("Set PWM pin to " + String(i));
    if (i % frequency == 0) { mlog("Read sensors and store data with index " + String(i/frequency)); }
    if (i % (4 * frequency) == 0) { mlog("Set LED pin to " + String((i % (8 * frequency)) / (4 * frequency))); }
    wait(duration * 1000 / 255);
  }
  mlog("Set PWM pin to 0.");
  mlog("Set LED pin to 0.");
  mlog("Write " + String(255 / frequency) + " lines of data to DATA.TXT.");
}

void reset() {
/*  pinMode(2, OUTPUT);
  digitalWrite(2, LOW);
  pinMode(7, INPUT_PULLUP);
  delay(10);
  if (digitalRead(7) != LOW) { return; }
  
  EEPROM.write(stateAddress, 0);
  EEPROM.write(counterAddress, 0);
  for (int i=0; i<10; i++) {
    digitalWrite(ledPin, HIGH);
    delay(50);
    digitalWrite(ledPin, LOW);
    delay(50);
  }
  delay(5000);*/
}

void wait(int n) {
//  delay(n);
  timestamp += n;
}

// SD CARD AND EEPROM FUNCTIONS *****************************************************************************************

void mlog(String data) {
  Serial.print(timestamp);
  Serial.print(" > ");
  Serial.println(data);
}

void logBackupData() {
/*  for (int i=0; i<40; i++) {
    EEPROM.write(10 + i*8, (data2[i] + 4096) % 256);
    EEPROM.write(11 + i*8, (data2[i] + 4096) / 256);
    EEPROM.write(12 + i*8, (data4[i] + 4096) % 256);
    EEPROM.write(13 + i*8, (data4[i] + 4096) / 256);
    EEPROM.write(14 + i*8, data5[i] % 256);
    EEPROM.write(15 + i*8, data5[i] / 256);
    EEPROM.write(16 + i*8, data6[i] % 256);
    EEPROM.write(17 + i*8, data6[i] / 256);
  }*/
  backupData = true;
}

void writeDataToLog(int n) {  
/*  SD.begin(chipSelect);
  File dataFile = SD.open("data.txt", FILE_WRITE);
  if (dataFile) {
    for (int i=0; i<=n; i++) {
      dataFile.print(data1[i]);
      dataFile.print(",");
      dataFile.print(data2[i]);
      dataFile.print(",");
      dataFile.print(data3[i]);
      dataFile.print(",");
      dataFile.print(data4[i]);
      dataFile.print(",");
      dataFile.print(data5[i]);
      dataFile.print(",");
      dataFile.print(data6[i]);
      dataFile.println(",");
    }
    dataFile.close();
  }*/
}
