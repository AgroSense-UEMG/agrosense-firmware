#include <Arduino.h>
#define LED_PIN 2

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  Serial.println("AgroSense Node-Six: Boot Inicializado! 🚀");
}

void loop() {
  Serial.println("Status: Aguardando sensores...");
  digitalWrite(LED_PIN, HIGH);
  delay(1000);
  digitalWrite(LED_PIN, LOW);
  delay(1000);
}
