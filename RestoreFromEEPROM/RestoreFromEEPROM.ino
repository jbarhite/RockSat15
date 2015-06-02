#include <SD.h>
#include <EEPROM.h>

#define chipSelect 8
String output;

void setup() {
  restore();
}

void loop() {
  
}

void restore() {
  SD.begin(chipSelect);
  File file = SD.open("restored.txt", FILE_WRITE);
  if (file) {
    for (int i=0; i<40; i++) {
      output = String(EEPROM.read(10 + i*8) + 256 * EEPROM.read(11 + i*8) - 4096);
      output += ",";
      output += String(EEPROM.read(12 + i*8) + 256 * EEPROM.read(13 + i*8) - 4096);
      output += ",";
      output += String(EEPROM.read(14 + i*8) + 256 * EEPROM.read(15 + i*8));
      output += ",";
      output += String(EEPROM.read(16 + i*8) + 256 * EEPROM.read(17 + i*8));
      output += ",";
      file.println(output);
    }
    file.close();
  }
}
