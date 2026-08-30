#include <Wire.h>

#define SDA_PIN 21
#define SCL_PIN 22

void setup() {
  Serial.begin(115200);

  Wire.begin(SDA_PIN, SCL_PIN);

  Serial.println("MPU6050 Test Started");

  Wire.beginTransmission(0x68);
  byte error = Wire.endTransmission();

  if (error == 0) {
    Serial.println("MPU6050 FOUND!");
  } else {
    Serial.println("MPU6050 NOT FOUND!");
  }
}

void loop() {
  Wire.beginTransmission(0x68);
  Wire.write(0x3B);
  Wire.endTransmission(false);

  Wire.requestFrom(0x68, 6);

  int16_t ax = Wire.read() << 8 | Wire.read();
  int16_t ay = Wire.read() << 8 | Wire.read();
  int16_t az = Wire.read() << 8 | Wire.read();

  Serial.print("X: ");
  Serial.print(ax);
  Serial.print("  Y: ");
  Serial.print(ay);
  Serial.print("  Z: ");
  Serial.println(az);

  delay(500);
}