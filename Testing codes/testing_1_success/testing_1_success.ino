#include <Wire.h>
#include <SPI.h>
#include <LoRa.h>
#include <DHT.h>

// =====================================================
// PIN DEFINITIONS
// =====================================================

// MPU6050
#define SDA_PIN 21
#define SCL_PIN 22

// DHT11
#define DHT_PIN 4
#define DHT_TYPE DHT11

// SW-420
#define VIBRATION_PIN 27

// LED
#define LED_PIN 2

// LoRa SX1278
#define SS   5
#define RST  14
#define DIO0 26

// =====================================================
// OBJECTS
// =====================================================

DHT dht(DHT_PIN, DHT_TYPE);

// =====================================================
// PACKET VARIABLES
// =====================================================

unsigned long packetNumber = 0;

// MPU6050 raw values
int16_t ax = 0;
int16_t ay = 0;
int16_t az = 0;

// Calculated tilt
float tiltX = 0.0;
float tiltY = 0.0;

// DHT11
float temperature = 0.0;
float humidity = 0.0;

// SW420
int vibrationState = 0;

// =====================================================
// SETUP
// =====================================================

void setup()
{
  Serial.begin(115200);
  delay(2000);

  Serial.println();
  Serial.println("==============================================");
  Serial.println("       SIH26025 SENSOR NODE - NODE 01");
  Serial.println("==============================================");

  // -------------------------------------------------
  // LED
  // -------------------------------------------------

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // -------------------------------------------------
  // SW-420
  // -------------------------------------------------

  pinMode(VIBRATION_PIN, INPUT);

  // -------------------------------------------------
  // MPU6050
  // -------------------------------------------------

  Wire.begin(SDA_PIN, SCL_PIN);

  Serial.println();
  Serial.println("Checking MPU6050...");

  Wire.beginTransmission(0x68);
  byte error = Wire.endTransmission();

  if (error == 0)
  {
    Serial.println("MPU6050 FOUND!");
  }
  else
  {
    Serial.println("MPU6050 NOT FOUND!");
  }

  // -------------------------------------------------
  // DHT11
  // -------------------------------------------------

  dht.begin();

  Serial.println("DHT11 started.");

  // -------------------------------------------------
  // LoRa SX1278
  // -------------------------------------------------

  Serial.println();
  Serial.println("Starting LoRa SX1278...");

  LoRa.setPins(SS, RST, DIO0);

  if (!LoRa.begin(433E6))
  {
    Serial.println("LoRa NOT detected!");
    Serial.println("Check SX1278 wiring.");
    
    // Don't stop the program.
    // We still want to test the sensors.
  }
  else
  {
    Serial.println("LoRa detected!");
    Serial.println("LoRa frequency: 433 MHz");
  }

  Serial.println();
  Serial.println("==============================================");
  Serial.println("       SENSOR NODE READY");
  Serial.println("==============================================");
}

// =====================================================
// READ MPU6050
// =====================================================

void readMPU6050()
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

// =====================================================
// CALCULATE TILT
// =====================================================

void calculateTilt()
{
  // Convert raw accelerometer values to approximate
  // acceleration values using ±2g sensitivity.

  float accX = ax / 16384.0;
  float accY = ay / 16384.0;
  float accZ = az / 16384.0;

  tiltX = atan2(
            accY,
            sqrt(accX * accX + accZ * accZ)
          ) * 180.0 / PI;

  tiltY = atan2(
            -accX,
            sqrt(accY * accY + accZ * accZ)
          ) * 180.0 / PI;
}

// =====================================================
// CREATE AND DISPLAY PACKET
// =====================================================

void createPacket()
{
  packetNumber++;

  Serial.println();
  Serial.println("--------------- SENSOR PACKET ---------------");

  Serial.print("NODE_ID       : NODE_01");
  Serial.println();

  Serial.print("PACKET_NO     : ");
  Serial.println(packetNumber);

  Serial.print("TILT_X        : ");
  Serial.print(tiltX, 2);
  Serial.println(" deg");

  Serial.print("TILT_Y        : ");
  Serial.print(tiltY, 2);
  Serial.println(" deg");

  Serial.print("ACCEL_X_RAW   : ");
  Serial.println(ax);

  Serial.print("ACCEL_Y_RAW   : ");
  Serial.println(ay);

  Serial.print("ACCEL_Z_RAW   : ");
  Serial.println(az);

  Serial.print("TEMPERATURE   : ");

  if (isnan(temperature))
  {
    Serial.println("ERROR");
  }
  else
  {
    Serial.print(temperature, 1);
    Serial.println(" C");
  }

  Serial.print("HUMIDITY      : ");

  if (isnan(humidity))
  {
    Serial.println("ERROR");
  }
  else
  {
    Serial.print(humidity, 1);
    Serial.println(" %");
  }

  Serial.print("VIBRATION     : ");

  if (vibrationState == HIGH)
  {
    Serial.println("DETECTED");
  }
  else
  {
    Serial.println("NORMAL");
  }

  Serial.println("----------------------------------------------");
}

// =====================================================
// LOOP
// =====================================================

void loop()
{
  // -------------------------------------------------
  // Read MPU6050
  // -------------------------------------------------

  readMPU6050();

  // -------------------------------------------------
  // Calculate tilt
  // -------------------------------------------------

  calculateTilt();

  // -------------------------------------------------
  // Read DHT11
  // -------------------------------------------------

  temperature = dht.readTemperature();
  humidity = dht.readHumidity();

  // -------------------------------------------------
  // Read SW-420
  // -------------------------------------------------

  vibrationState = digitalRead(VIBRATION_PIN);

  // -------------------------------------------------
  // LED
  // -------------------------------------------------

  if (vibrationState == HIGH)
  {
    digitalWrite(LED_PIN, HIGH);
  }
  else
  {
    digitalWrite(LED_PIN, LOW);
  }

  // -------------------------------------------------
  // Create packet
  // -------------------------------------------------

  createPacket();

  // -------------------------------------------------
  // Wait
  // -------------------------------------------------

  delay(2000);
}