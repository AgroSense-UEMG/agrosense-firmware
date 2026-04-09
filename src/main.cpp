#include <Arduino.h>
#include "WiFiManager.h"
#include "EnvironmentSensor.h"

#define LED_PIN 2

unsigned long previousMillis = 0;
int ledState = LOW;
EnvironmentSensor environmentSensor(4, DHT11);
WiFiManager wifiManager;

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  environmentSensor.begin();
  Serial.println("AgroSense Node-Six: Boot Inicializado! 🚀");
  wifiManager.begin();
}

void loop() {
  unsigned long currentMillis = millis();
  wifiManager.loop();

  if (currentMillis - previousMillis >= 2000UL) {
    previousMillis = currentMillis;
    ledState = (ledState == LOW ? HIGH : LOW);
    digitalWrite(LED_PIN, ledState);
    Serial.print("LED ");
    Serial.println(ledState == HIGH ? "ON" : "OFF");

    EnvironmentSensor::EnvData reading = environmentSensor.readData();
    if (reading.valid) {
      Serial.print("Temperatura: ");
      Serial.print(reading.temperature, 1);
      Serial.print(" C, Umidade: ");
      Serial.print(reading.humidity, 1);
      Serial.println(" %");
    } else {
      Serial.println("Leitura ambiente inválida.");
    }
  }
}
