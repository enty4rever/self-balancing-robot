#include <Arduino.h>
#include <Wire.h>

#define SDA_PIN 21
#define SCL_PIN 22

#define MPU_ADD 0x68
#define PWR_MGMT_1 0x6B
#define ACC_X_H 0x3B

float filtered_angle = 0.0;

unsigned long time_prev = 0;

const float alpha = 0.98; // Complementary filter coefficient

void setup() {
  Serial.begin(115200);
  delay(1000); 
  
  Wire.begin(SDA_PIN, SCL_PIN); 
   
  Wire.beginTransmission(MPU_ADD);
  Wire.write(PWR_MGMT_1);
  Wire.write(0);
  Wire.endTransmission(true);

  time_prev = millis();
}

void loop() {
  Wire.beginTransmission(MPU_ADD);
  Wire.write(ACC_X_H);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADD, 14, true);

  int16_t acc_x = (Wire.read() << 8) | Wire.read();
  int16_t acc_y = (Wire.read() << 8) | Wire.read();
  int16_t acc_z = (Wire.read() << 8) | Wire.read();
  int16_t temp = (Wire.read() << 8) | Wire.read();
  int16_t gy_x = (Wire.read() << 8) | Wire.read();
  int16_t gy_y = (Wire.read() << 8) | Wire.read();
  int16_t gy_z = (Wire.read() << 8) | Wire.read();

  float a_x = acc_x / 16384.0;
  float a_z = acc_z / 16384.0; // Convert to g(s)
  float g_y = gy_y / 131.0; // Convert to deg/s

  unsigned long time_now = millis();
  float dt = (time_now - time_prev) / 1000.0; // Convert to second(s)
  time_prev = time_now;

  float angle_acc = atan2(a_x, a_z) * (180.0 / PI);

  filtered_angle = alpha * (filtered_angle + g_y * dt) + (1 - alpha) * angle_acc;

  Serial.print("Accelerometer Angle: ");
  Serial.println(angle_acc);
  Serial.print("Gyroscope Rate: ");
  Serial.println(g_y);
  Serial.print("Filtered Angle: ");
  Serial.println(filtered_angle);

  delay(10);
}