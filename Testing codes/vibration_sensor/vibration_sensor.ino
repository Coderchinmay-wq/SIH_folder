#define VIBRATION_PIN 27

void setup() {
  Serial.begin(115200);
  pinMode(VIBRATION_PIN, INPUT);

  Serial.println("Vibration Sensor Test Started");
}

void loop() {
  int state = digitalRead(VIBRATION_PIN);

  if (state == HIGH) {
    Serial.println("VIBRATION DETECTED!");
  } else {
    Serial.println("No vibration");
  }

  delay(200);
}