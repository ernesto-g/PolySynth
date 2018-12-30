#include <Wire.h>

//#include "24LC256.h"
//#define EEPROM_ADDRESS 0x50 // 24LC256 EEPROM Address in i2c bus 
//static E24LC256 mem(EEPROM_ADDRESS);  

#define EEPROM_ADDR 0x50


void EEPROM_write(unsigned int addr,byte data) {
  int rdata = data;
  Wire.beginTransmission(EEPROM_ADDR);
  Wire.write((int)(addr >> 8));       // MSB
  Wire.write((int)(addr & 0xFF));     // LSB
  Wire.write(rdata);
  Wire.endTransmission();
  //Serial.print("EEPROM write: addr: ");
  //Serial.print(addr);
  //Serial.print(" ");
  //Serial.println(data);
  delay(5);
}

byte EEPROM_read(unsigned int addr) {
  byte data = 0xFF;
  Wire.beginTransmission(EEPROM_ADDR);
  Wire.write((int)(addr >> 8));       // MSB
  Wire.write((int)(addr & 0xFF));     // LSB
  Wire.endTransmission();
  Wire.requestFrom(EEPROM_ADDR,1);
  if (Wire.available()) data = Wire.read();
  //Serial.print("EEPROM read: addr: ");
  //Serial.print(addr);
  //Serial.print(" ");
  //Serial.println(data);
  delay(5);
  return data;
}


void mem_init(void)
{
    Wire.begin();
  
    unsigned char c = EEPROM_read(0x55);
    Serial.print("LEI BYTE:");
    Serial.print(c,DEC);
    Serial.print("\n");

    
    //Serial.print("Escribo...\n");
    //EEPROM_write(0x55, 0x27);
    //Serial.print("Escribi\n");
    
}

