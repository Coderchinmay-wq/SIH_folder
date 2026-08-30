#include <SPI.h>
#include <LoRa.h>

#define SS   5
#define RST  14
#define DIO0 26

void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println("ESP32 started");
  Serial.println("Starting LoRa...");

  LoRa.setPins(SS, RST, DIO0);

  if (!LoRa.begin(433E6)) {
    Serial.println("LoRa NOT detected!");
    while (1);
  }

  Serial.println("LoRa detected!");
}

void loop() {
}