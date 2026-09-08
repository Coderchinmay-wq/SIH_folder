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

// LoRa frequency
#define LORA_FREQUENCY 433E6

// =====================================================
// OBJECTS
// =====================================================

DHT dht(DHT_PIN, DHT_TYPE);

// =====================================================
// VARIABLES
// =====================================================

unsigned long packetNumber = 0;

// MPU6050 raw values
int16_t ax = 0;
int16_t ay = 0;
int16_t az = 0;

// Tilt values
float tiltX = 0.0;
float tiltY = 0.0;

// DHT11
float temperature = 0.0;
float humidity = 0.0;

// SW-420
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

  Serial.println();
  Serial.println("SW-420 initialized.");

  // -------------------------------------------------
  // MPU6050
  // -------------------------------------------------

  Serial.println();
  Serial.println("Initializing MPU6050...");

  Wire.begin(SDA_PIN, SCL_PIN);

  Wire.beginTransmission(0x68);
  byte error = Wire.endTransmission();

  if (error == 0)
  {
    Serial.println("MPU6050 FOUND!");

    // Wake up MPU6050
    Wire.beginTransmission(0x68);
    Wire.write(0x6B);
    Wire.write(0x00);
    Wire.endTransmission();

    Serial.println("MPU6050 initialized.");
  }
  else
  {
    Serial.println("MPU6050 NOT FOUND!");
    Serial.println("Check SDA/SCL wiring.");
  }

  // -------------------------------------------------
  // DHT11
  // -------------------------------------------------

  Serial.println();
  Serial.println("Initializing DHT11...");

  dht.begin();

  Serial.println("DHT11 initialized.");

  // -------------------------------------------------
  // LoRa SX1278
  // -------------------------------------------------

  Serial.println();
  Serial.println("Initializing LoRa SX1278...");

  LoRa.setPins(SS, RST, DIO0);

  if (!LoRa.begin(LORA_FREQUENCY))
  {
    Serial.println("LoRa initialization FAILED!");
    Serial.println("Check SX1278 wiring and power.");

    while (1)
    {
      digitalWrite(LED_PIN, !digitalRead(LED_PIN));
      delay(500);
    }
  }

  Serial.println("LoRa initialization SUCCESS!");
  Serial.println("Frequency : 433 MHz");

  // -------------------------------------------------
  // LoRa settings
  // -------------------------------------------------

  LoRa.setTxPower(17);
  LoRa.setSpreadingFactor(7);
  LoRa.setSignalBandwidth(125E3);
  LoRa.setCodingRate4(5);

  Serial.println("Spreading Factor : SF7");
  Serial.println("Bandwidth        : 125 kHz");
  Serial.println("Coding Rate      : 4/5");
  Serial.println("TX Power         : 17 dBm");

  // -------------------------------------------------
  // READY
  // -------------------------------------------------

  Serial.println();
  Serial.println("==============================================");
  Serial.println("          SENSOR NODE READY");
  Serial.println("==============================================");
  Serial.println();
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
    ax = (Wire.read() << 8) | Wire.read();
    ay = (Wire.read() << 8) | Wire.read();
    az = (Wire.read() << 8) | Wire.read();
  }
}

// =====================================================
// CALCULATE TILT
// =====================================================

void calculateTilt()
{
  float accX = ax / 16384.0;
  float accY = ay / 16384.0;
  float accZ = az / 16384.0;

  tiltX = atan2(
            accY,
            sqrt((accX * accX) + (accZ * accZ))
          ) * 180.0 / PI;

  tiltY = atan2(
            -accX,
            sqrt((accY * accY) + (accZ * accZ))
          ) * 180.0 / PI;
}

// =====================================================
// READ SENSORS
// =====================================================

void readSensors()
{
  // MPU6050
  readMPU6050();
  calculateTilt();

  // DHT11
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();

  // SW-420
  vibrationState = digitalRead(VIBRATION_PIN);

  // LED
  if (vibrationState == HIGH)
  {
    digitalWrite(LED_PIN, HIGH);
  }
  else
  {
    digitalWrite(LED_PIN, LOW);
  }
}

// =====================================================
// SEND LoRa PACKET
// =====================================================

void sendLoRaPacket()
{
  packetNumber++;

  // -------------------------------------------------
  // Handle DHT11 errors
  // -------------------------------------------------

  if (isnan(temperature))
  {
    temperature = -999.0;
  }

  if (isnan(humidity))
  {
    humidity = -999.0;
  }

  // -------------------------------------------------
  // Create packet
  // -------------------------------------------------

  String packet = "";

  packet += "NODE=NODE_01";
  packet += ",PKT=";
  packet += packetNumber;

  packet += ",TX=";
  packet += String(tiltX, 2);

  packet += ",TY=";
  packet += String(tiltY, 2);

  packet += ",AX=";
  packet += ax;

  packet += ",AY=";
  packet += ay;

  packet += ",AZ=";
  packet += az;

  packet += ",TEMP=";
  packet += String(temperature, 1);

  packet += ",HUM=";
  packet += String(humidity, 1);

  packet += ",VIB=";
  packet += vibrationState;

  // -------------------------------------------------
  // Display packet
  // -------------------------------------------------

  Serial.println();
  Serial.println("----------------------------------------------");
  Serial.println("SENSOR DATA");

  Serial.print("Node        : NODE_01");
  Serial.println();

  Serial.print("Packet No   : ");
  Serial.println(packetNumber);

  Serial.print("Tilt X      : ");
  Serial.print(tiltX, 2);
  Serial.println(" deg");

  Serial.print("Tilt Y      : ");
  Serial.print(tiltY, 2);
  Serial.println(" deg");

  Serial.print("Accel X     : ");
  Serial.println(ax);

  Serial.print("Accel Y     : ");
  Serial.println(ay);

  Serial.print("Accel Z     : ");
  Serial.println(az);

  Serial.print("Temperature : ");

  if (temperature == -999.0)
  {
    Serial.println("ERROR");
  }
  else
  {
    Serial.print(temperature, 1);
    Serial.println(" C");
  }

  Serial.print("Humidity    : ");

  if (humidity == -999.0)
  {
    Serial.println("ERROR");
  }
  else
  {
    Serial.print(humidity, 1);
    Serial.println(" %");
  }

  Serial.print("Vibration   : ");

  if (vibrationState == HIGH)
  {
    Serial.println("DETECTED");
  }
  else
  {
    Serial.println("NORMAL");
  }

  // -------------------------------------------------
  // Transmit LoRa packet
  // -------------------------------------------------

  Serial.println();
  Serial.println("Sending LoRa packet...");

  LoRa.beginPacket();

  LoRa.print(packet);

  int result = LoRa.endPacket();

  if (result == 1)
  {
    Serial.println("LoRa packet SENT successfully!");
  }
  else
  {
    Serial.println("LoRa packet transmission FAILED!");
  }

  Serial.println("----------------------------------------------");
}

// =====================================================
// MAIN LOOP
// =====================================================

void loop()
{
  // Read all sensors
  readSensors();

  // Send data through LoRa
  sendLoRaPacket();

  // Wait 2 seconds
  delay(2000);
}