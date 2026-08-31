#include <Wire.h>
#include <SPI.h>
#include <LoRa.h>
#include <DHT.h>

// =============================
// PIN DEFINITIONS
// =============================

#define SDA_PIN 21
#define SCL_PIN 22

#define DHT_PIN 4
#define DHT_TYPE DHT11

#define VIBRATION_PIN 27
#define LED_PIN 2

// LoRa
#define SS   5
#define RST  14
#define DIO0 26

// =============================
// DHT
// =============================

DHT dht(DHT_PIN, DHT_TYPE);

// =============================
// VARIABLES
// =============================

unsigned long packetNumber = 0;

int16_t ax, ay, az;

float tiltX;
float tiltY;

float temperature;
float humidity;

int vibrationState;

// =============================
// SETUP
// =============================

void setup()
{
  Serial.begin(115200);
  delay(1000);

  pinMode(LED_PIN, OUTPUT);
  pinMode(VIBRATION_PIN, INPUT);

  // MPU6050 I2C
  Wire.begin(SDA_PIN, SCL_PIN);

  // Check MPU6050
  Wire.beginTransmission(0x68);

  if (Wire.endTransmission() == 0)
  {
    Serial.println("MPU6050 OK");
  }
  else
  {
    Serial.println("MPU6050 ERROR");
  }

  // DHT
  dht.begin();

  // LoRa
  LoRa.setPins(SS, RST, DIO0);

  Serial.println("Starting LoRa...");

  if (!LoRa.begin(433E6))
  {
    Serial.println("LoRa initialization FAILED!");

    while (1)
    {
      digitalWrite(LED_PIN, HIGH);
      delay(200);
      digitalWrite(LED_PIN, LOW);
      delay(200);
    }
  }

  Serial.println("LoRa initialization SUCCESS!");

  Serial.println("NODE 01 READY");
}

// =============================
// READ MPU6050
// =============================

void readMPU()
{
  Wire.beginTransmission(0x68);

  Wire.write(0x3B);

  Wire.endTransmission(false);

  Wire.requestFrom(0x68, 6);

  if (Wire.available() >= 6)
  {
    ax = Wire.read() << 8 | Wire.read();
    ay = Wire.read() << 8 | Wire.read();
    az = Wire.read() << 8 | Wire.read();
  }
}

// =============================
// LOOP
// =============================

void loop()
{
  // Read MPU
  readMPU();

  float accX = ax / 16384.0;
  float accY = ay / 16384.0;
  float accZ = az / 16384.0;

  // Tilt
  tiltX = atan2(
    accY,
    sqrt(accX * accX + accZ * accZ)
  ) * 180.0 / PI;

  tiltY = atan2(
    -accX,
    sqrt(accY * accY + accZ * accZ)
  ) * 180.0 / PI;

  // DHT
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();

  // Keep previous valid values if DHT fails
  static float lastTemp = 0;
  static float lastHumidity = 0;

  if (!isnan(temperature))
    lastTemp = temperature;
  else
    temperature = lastTemp;

  if (!isnan(humidity))
    lastHumidity = humidity;
  else
    humidity = lastHumidity;

  // SW420
  vibrationState = digitalRead(VIBRATION_PIN);

  // Packet number
  packetNumber++;

  // =============================
  // CREATE PACKET
  // =============================

  String packet = "";

  packet += "N01";
  packet += ",";
  packet += String(packetNumber);
  packet += ",";
  packet += String(tiltX, 2);
  packet += ",";
  packet += String(tiltY, 2);
  packet += ",";
  packet += String(accX, 2);
  packet += ",";
  packet += String(accY, 2);
  packet += ",";
  packet += String(accZ, 2);
  packet += ",";
  packet += String(temperature, 1);
  packet += ",";
  packet += String(humidity, 1);
  packet += ",";
  packet += String(vibrationState);

  // =============================
  // TRANSMIT
  // =============================

  Serial.println();
  Serial.println("TRANSMITTING:");
  Serial.println(packet);

  LoRa.beginPacket();
  LoRa.print(packet);
  LoRa.endPacket();

  Serial.println("Packet sent.");

  digitalWrite(LED_PIN, HIGH);
  delay(100);
  digitalWrite(LED_PIN, LOW);

  delay(2000);
}
