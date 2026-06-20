#include <Arduino.h>
#include <Wire.h>

#define SDA_PIN 21
#define SCL_PIN 22

void setup() {
  Serial.begin(115200);
  delay(1000); 
  
  Wire.begin(SDA_PIN, SCL_PIN); 
   
  byte count = 0;

  for (byte address = 1; address < 127; address++)
  {
    Wire.beginTransmission(address);
    byte result = Wire.endTransmission();

    if (result == 0) 
    {
      Serial.print("I2C device found at address 0x");
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
      count++;
    }
  }


  if (count == 0) 
  {
    Serial.println("No I2C devices found");
  } 
  else 
  {
   Serial.print(count);
    Serial.println(" I2C devices found");
  }
}

void loop() {
  
}